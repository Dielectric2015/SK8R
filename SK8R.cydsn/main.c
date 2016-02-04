/* ========================================
 *
 * Skater Lights.  That's how I roll.
 * No warranty expressed or implied.
 *
 * ========================================
*/
#include <project.h>

#define FPS_SCALE 6867

#define PURPLE 0x00808000
#define VIOLET 0x00FF5500
#define BLUE   0x00FF0000
#define RED    0x0000FF00
#define ORANGE 0x0000FF32
#define YELLOW 0x00008055

//just for I2C monitoring on the bridge software
struct xferstruct
{
    uint8 I2C_Buffer[4];
} I2C_Struct;

char Timer_Capture_Flag, NoSpeed;
uint16 Timer_Capture, Timer_Capture_Last;
uint8 PixelMask, PixelCounter;
uint32 Timeout;

CY_ISR_PROTO(Timer_ISR);
void StepColors(void);

int main()
{
    uint16 Diff, FPS;
   
    CyGlobalIntEnable; /* Enable global interrupts. */

    PixelMask = 0;
    PixelCounter = 0;
    NoSpeed = 0;
    
    Timer_Start();
    Timer_ISR_StartEx(Timer_ISR);
    
    StripLights_Start();

    StripLights_Dim(0);
    StripLights_MemClear(0x00000000);
    StripLights_Trigger(1);

//    EZI2C_EzI2CSetBuffer1(sizeof(I2C_Struct.I2C_Buffer), sizeof(I2C_Struct.I2C_Buffer), I2C_Struct.I2C_Buffer);
//    EZI2C_Start();
    
    Diff = 60;
    
    for(;;)
    {
        //any Hall captures yet?
        if(Timer_Capture_Flag)
        {
            if(Timer_Capture > Timer_Capture_Last)
                Diff = Timer_Capture - Timer_Capture_Last;
            else
                Diff = (65535 - Timer_Capture_Last) + Timer_Capture;
            
            Timer_Capture_Flag = 0;
            StepColors();
            NoSpeed = 0;
        }
        //nope, maybe I stopped
        else
        {
            Timeout++;
        }
        //kill the lights if I'm standing still
        if(Timeout > 20000)
        {
            NoSpeed = 1;
            Timeout = 0;
        }
        
        FPS = FPS_SCALE/Diff;   //not using this but it could be fun later
        
//        I2C_Struct.I2C_Buffer[0] = (uint8)(Diff >>8);
//        I2C_Struct.I2C_Buffer[1] = (uint8)Diff;
//        I2C_Struct.I2C_Buffer[2] = (uint8)(FPS >>8);
//        I2C_Struct.I2C_Buffer[3] = (uint8)FPS;
//        if (!(EZI2C_EzI2CGetActivity() & EZI2C_EZI2C_STATUS_BUSY))
//        {
//            I2C_Struct.Capture = Diff;
//        }

        //OK, time to write out to the LEDs
        if(1 == NoSpeed)
            StripLights_MemClear(0x00000000);
        StripLights_Trigger(1);
        while(!StripLights_Ready());
    }
}

CY_ISR(Timer_ISR)
{
    Timer_ClearInterrupt(Timer_CAPTURE_MASK);
    Blue_LED_Write(~Blue_LED_Read());
    //Timer_Capture_Last = Timer_Capture;
    Timer_Capture = Timer_ReadCapture();
    Timer_Capture_Last = Timer_ReadCaptureBuf();
    Timer_Capture_Flag = 1;
}

//yay, new input!  Step the colors
void StepColors(void){
    //PixelCounter is used as the main index into the strip's pixel array
    //PixelMask is there because this particular installation is "reversed"
    //so we need to start the motion from the far end of the strip
    if(PixelCounter < 27)
        PixelCounter++;
    else PixelCounter = 0;
    PixelMask = 27-PixelCounter;

    StripLights_MemClear(0x00000000);
    
    StripLights_Pixel(PixelMask   ,0,PURPLE);
    StripLights_Pixel(PixelMask -1,0,VIOLET);
    StripLights_Pixel(PixelMask -2,0,BLUE);
    StripLights_Pixel(PixelMask -3,1,YELLOW);
    StripLights_Pixel(PixelMask -4,1,ORANGE);
    StripLights_Pixel(PixelMask -5,1,RED);
}
/* [] END OF FILE */
