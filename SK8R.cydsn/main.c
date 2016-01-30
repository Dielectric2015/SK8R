/* ========================================
 *
 * Skate or Die!
 *
 * ========================================
*/
#include <project.h>

#define FPS_SCALE 6867

#define PURPLE 0x00808000
#define VIOLET 0x00FF5500
#define BLUE   0x00FF0000

struct xferstruct
{
    uint8 I2C_Buffer[4];
} I2C_Struct;

char Timer_Capture_Flag, NoSpeed;
uint16 Timer_Capture, Timer_Capture_Last;

uint32 Timeout, Timeout1;
CY_ISR_PROTO(Timer_ISR);

int main()
{
    uint16 Diff, FPS;
    uint8 PixelMask, PixelCounter;
    CyGlobalIntEnable; /* Enable global interrupts. */

    Timer_Start();
    Timer_ISR_StartEx(Timer_ISR);
    
    StripLights_Start();

    StripLights_Dim(0);
    StripLights_MemClear(0x00000000);
    StripLights_Trigger(1);
    //CyDelay(100);
    EZI2C_EzI2CSetBuffer1(sizeof(I2C_Struct.I2C_Buffer), sizeof(I2C_Struct.I2C_Buffer), I2C_Struct.I2C_Buffer);
    EZI2C_Start();
    
    Diff = 60;
    
    for(;;)
    {
        if(Timer_Capture_Flag)
        {
            if(Timer_Capture > Timer_Capture_Last)
                Diff = Timer_Capture - Timer_Capture_Last;
            else
                Diff = (65535 - Timer_Capture_Last) + Timer_Capture;
            
            Timer_Capture_Flag = 0;
            Timeout1++;
            NoSpeed = 0;
        }
        else
        {
            Timeout++;
        }
        if(Timeout > 20000)
        {
            NoSpeed = 1;
            Timeout = 0;
        }
        
        
        
        FPS = FPS_SCALE/Diff;
        
        I2C_Struct.I2C_Buffer[0] = (uint8)(Diff >>8);
        I2C_Struct.I2C_Buffer[1] = (uint8)Diff;
        I2C_Struct.I2C_Buffer[2] = (uint8)(FPS >>8);
        I2C_Struct.I2C_Buffer[3] = (uint8)FPS;
//        if (!(EZI2C_EzI2CGetActivity() & EZI2C_EZI2C_STATUS_BUSY))
//        {
//            I2C_Struct.Capture = Diff;
//        }
//        
        
        if(Timeout1) 
        {
            if(PixelCounter < 23)
                PixelCounter++;
            else PixelCounter = 0;
            PixelMask = 23-PixelCounter;
            Timeout1 = 0;
        }
        StripLights_MemClear(0x00000000);
        
        if(NoSpeed ==0)
        {
            StripLights_Pixel(PixelMask   ,0,PURPLE);
            StripLights_Pixel(PixelMask +1,0,VIOLET);
            StripLights_Pixel(PixelMask +2,0,BLUE);
            StripLights_Pixel(PixelMask +6,1,0x00008055);
            StripLights_Pixel(PixelMask +7,1,0x0000FF32);
            StripLights_Pixel(PixelMask +8,1,0x0000FF00);
        }    
        
        //CyDelay(10);
        StripLights_Trigger(1);
        while(!StripLights_Ready());
    }
}

CY_ISR(Timer_ISR)
{
    Timer_ClearInterrupt(Timer_CAPTURE_MASK);
    Red_LED_Write(~Red_LED_Read());
    //Timer_Capture_Last = Timer_Capture;
    Timer_Capture = Timer_ReadCapture();
    Timer_Capture_Last = Timer_ReadCaptureBuf();
    Timer_Capture_Flag = 1;
}
/* [] END OF FILE */
