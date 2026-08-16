// REGISTER DESCRIPTION
#define PAC7620_VAL(val, maskbit)		( val << maskbit )
#define PAC7620_ADDR_BASE				0x00

// REGISTER BANK SELECT
#define PAC7620_REGITER_BANK_SEL		(PAC7620_ADDR_BASE + 0xEF)	//W

// REGISTER BANK 0
#define PAC7620_ADDR_SUSPEND_CMD		(PAC7620_ADDR_BASE + 0x3)	//W
#define PAC7620_ADDR_GES_PS_DET_MASK_0		(PAC7620_ADDR_BASE + 0x41)	//RW
#define PAC7620_ADDR_GES_PS_DET_MASK_1		(PAC7620_ADDR_BASE + 0x42)	//RW
#define PAC7620_ADDR_GES_PS_DET_FLAG_0		(PAC7620_ADDR_BASE + 0x43)	//R
#define PAC7620_ADDR_GES_PS_DET_FLAG_1		(PAC7620_ADDR_BASE + 0x44)	//R
#define PAC7620_ADDR_STATE_INDICATOR	(PAC7620_ADDR_BASE + 0x45)	//R
#define PAC7620_ADDR_PS_HIGH_THRESHOLD	(PAC7620_ADDR_BASE + 0x69)	//RW
#define PAC7620_ADDR_PS_LOW_THRESHOLD	(PAC7620_ADDR_BASE + 0x6A)	//RW
#define PAC7620_ADDR_PS_APPROACH_STATE	(PAC7620_ADDR_BASE + 0x6B)	//R
#define PAC7620_ADDR_PS_RAW_DATA		(PAC7620_ADDR_BASE + 0x6C)	//R

// REGISTER BANK 1
#define PAC7620_ADDR_PS_GAIN			(PAC7620_ADDR_BASE + 0x44)	//RW
#define PAC7620_ADDR_IDLE_S1_STEP_0		(PAC7620_ADDR_BASE + 0x67)	//RW
#define PAC7620_ADDR_IDLE_S1_STEP_1		(PAC7620_ADDR_BASE + 0x68)	//RW
#define PAC7620_ADDR_IDLE_S2_STEP_0		(PAC7620_ADDR_BASE + 0x69)	//RW
#define PAC7620_ADDR_IDLE_S2_STEP_1		(PAC7620_ADDR_BASE + 0x6A)	//RW
#define PAC7620_ADDR_OP_TO_S1_STEP_0	(PAC7620_ADDR_BASE + 0x6B)	//RW
#define PAC7620_ADDR_OP_TO_S1_STEP_1	(PAC7620_ADDR_BASE + 0x6C)	//RW
#define PAC7620_ADDR_OP_TO_S2_STEP_0	(PAC7620_ADDR_BASE + 0x6D)	//RW
#define PAC7620_ADDR_OP_TO_S2_STEP_1	(PAC7620_ADDR_BASE + 0x6E)	//RW
#define PAC7620_ADDR_OPERATION_ENABLE	(PAC7620_ADDR_BASE + 0x72)	//RW

// PAC7620_REGITER_BANK_SEL
#define PAC7620_BANK0		PAC7620_VAL(0,0)
#define PAC7620_BANK1	PAC7620_VAL(1,0)

// PAC7620_ADDR_SUSPEND_CMD
#define PAC7620_I2C_WAKEUP	PAC7620_VAL(1,0)
#define PAC7620_I2C_SUSPEND	PAC7620_VAL(0,0)

// PAC7620_ADDR_OPERATION_ENABLE
#define PAC7620_ENABLE		PAC7620_VAL(1,0)
#define PAC7620_DISABLE		PAC7620_VAL(0,0)

typedef enum {
	BANK0 = 0,
	BANK1,		
} bank_e;

enum {
	// REGISTER 0
	GES_RIGHT_FLAG			 = BIT(0),
	GES_LEFT_FLAG			 = BIT(1),
	GES_UP_FLAG				 = BIT(2),
	GES_DOWN_FLAG			 = BIT(3),
	GES_FORWARD_FLAG		 = BIT(4),
	GES_BACKWARD_FLAG		 = BIT(5),
	GES_CLOCKWISE_FLAG		 = BIT(6),
	GES_COUNT_CLOCKWISE_FLAG = BIT(7),
	//REGISTER 1
	GES_WAVE_FLAG		= BIT(0),	
};


#define PAJ7620U2_15cm
//#define PAJ7620U2_20cm
//#define PAJ7620U2_25cm

#ifdef PAJ7620U2_15cm
//Boy modify @ 2015_0923
unsigned char init_register_array[][2] = {	// Initial Gesture
	
	{0xEF,0x00},                                              
	{0x41,0xFF},//R_Int_1_En[7:0]                             
	{0x42,0x01},//R_Int_2_En[7:0]                             
	{0x46,0x2D},//R_AELedOff_UB[7:0]                          
	{0x47,0x0F},//R_AELedOff_LB[7:0]                          
	{0x48,0x80},//R_AE_Exposure_UB[7:0]                       
	{0x49,0x00},//R_AE_Exposure_UB[15:8]                      
	{0x4A,0x40},//R_AE_Exposure_LB[7:0]                       
	{0x4B,0x00},//R_AE_Exposure_LB[15:8]                      
	{0x4C,0x20},//R_AE_Gain_UB[7:0]                           
	{0x4D,0x00},//R_AE_Gain_LB[7:0]                           
	{0x51,0x10},//R_AE_EnH = 1                                
	{0x5C,0x02},//R_SenClkPrd[5:0]                            
	{0x5E,0x10},//R_SRAM_CLK_manual                           
	{0x80,0x42},//GPIO setting                                
	{0x81,0x44},//GPIO setting                                
	{0x82,0x0C},//Interrupt IO setting                        
	{0x83,0x20},//R_LightThd[7:0]                             
	{0x84,0x30},//R_ObjectSizeStartTh[7:0]                    
	{0x85,0x00},//R_ObjectSizeStartTh[9:8]                    
	{0x86,0x10},//R_ObjectSizeEndTh[7:0]                      
	{0x87,0x00},//R_ObjectSizeEndTh[9:8]                      
	{0x8B,0x01},//R_Cursor_ObjectSizeTh[7:0]                  
	{0x8D,0x00},//R_TimeDelayNum[7:0]                         
	{0x90,0x06},//R_NoMotionCountThd[6:0]                     
	{0x91,0x06},//R_NoObjectCountThd[6:0]                     
	{0x93,0x0D},//R_XDirectionThd[4:0]                        
	{0x94,0x0A},//R_YDirectionThd[4:0]                        
	{0x95,0x0A},//R_ZDirectionThd[4:0]                        
	{0x96,0x0C},//R_ZDirectionXYThd[4:0]                      
	{0x97,0x05},//R_ZDirectionAngleThd[3:0]                   
	{0x9A,0x14},//R_RotateXYThd[4:0]                          
	{0x9C,0x7F},//Filter setting                              
	{0x9F,0xF9},//R_UseBGModel is enable                      
	{0xA0,0x48},//R_BGUpdateMaxIntensity[7:0]                 
	{0xA5,0x19},//R_FilterAverage_Mode                        
	{0xCC,0x19},//R_YtoZSum[5:0]                              
	{0xCD,0x0B},//R_YtoZFactor[5:0]                           
	{0xCE,0x13},//bit[2:0] = R_PositionFilterLength[2:0]      
	{0xCF,0x62},//bit[3:0] = R_WaveCountThd[3:0]              
	{0xD0,0x21},//R_AbortXYRatio[4:0] & R_AbortLength[6:0]    
	{0xEF,0x01},                                              
	{0x00,0x1E},//Cmd_HSize[5:0]                              
	{0x01,0x1E},//Cmd_VSize[5:0]                              
	{0x02,0x0F},//Cmd_HStart[5:0]                             
	{0x03,0x0F},//Cmd_VStart[5:0]                             
	{0x04,0x02},//Sensor skip & flip                          
	{0x25,0x01},//R_LensShadingComp_EnH                       
	{0x26,0x00},//R_OffsetX[6:0]                              
	{0x27,0x39},//R_OffsetY[6:0]                              
	{0x28,0x7F},//R_LSC[6:0]                                  
	{0x29,0x08},//R_LSFT[3:0]                                 
	{0x30,0x03},//R_LED_SoftStart_time[7:0]                   
	{0x32,0x1A},//R_LED1_DAC_UB[4:0]                          
	{0x33,0x1A},//R_LED2_DAC_UB[4:0]                          
	{0x3E,0xFF},//Cmd_DebugPattern[7:0]                       
	{0x5E,0x3D},//analog voltage setting                      
	{0x65,0xAC},//R_IDLE_TIME[7:0] - 120fps  
	{0x66,0x00},//R_IDLE_TIME[15:8]                           
	{0x67,0x97},//R_IDLE_TIME_SLEEP_1[7:0]                    
	{0x68,0x01},//R_IDLE_TIME_SLEEP_1[15:8]                   
	{0x69,0xCD},//R_IDLE_TIME_SLEEP_2[7:0]                    
	{0x6A,0x01},//R_IDLE_TIME_SLEEP_2[15:8]                   
	{0x6B,0xB0},//R_Obj_TIME_1[7:0]                           
	{0x6C,0x04},//R_Obj_TIME_1[15:8]                          
	{0x6D,0x2C},//R_Obj_TIME_2[7:0]                           
	{0x6E,0x01},//R_Obj_TIME_2[15:8]                          
	{0x72,0x01},//R_TG_EnH                                    
	{0x73,0x35},//Auto Sleep & Wakeup mode                    
	{0x74,0x00},//R_Control_Mode[2:0]                         
	{0x77,0x01},//R_SRAM_Read_EnH
	{0xEF,0x00},   
};
#endif


#ifdef PAJ7620U2_20cm
//Boy modify @ 2015_0923
unsigned char init_register_array[][2] = {	// Initial Gesture
	
	{0xEF,0x00},                                              
	{0x41,0xFF},//R_Int_1_En[7:0]                             
	{0x42,0x01},//R_Int_2_En[7:0]                             
	{0x46,0x2D},//R_AELedOff_UB[7:0]                          
	{0x47,0x0F},//R_AELedOff_LB[7:0]                          
	{0x48,0x00},//R_AE_Exposure_UB[7:0]                       
	{0x49,0x01},//R_AE_Exposure_UB[15:8]                      
	{0x4A,0x80},//R_AE_Exposure_LB[7:0]                       
	{0x4B,0x00},//R_AE_Exposure_LB[15:8]                      
	{0x4C,0x20},//R_AE_Gain_UB[7:0]                           
	{0x4D,0x00},//R_AE_Gain_LB[7:0]                           
	{0x51,0x10},//R_AE_EnH = 1                                
	{0x5C,0x02},//R_SenClkPrd[5:0]                            
	{0x5E,0x10},//R_SRAM_CLK_manual                           
	{0x80,0x42},//GPIO setting                                
	{0x81,0x44},//GPIO setting                                
	{0x82,0x0C},//Interrupt IO setting                        
	{0x83,0x20},//R_LightThd[7:0]                             
	{0x84,0x30},//R_ObjectSizeStartTh[7:0]                    
	{0x85,0x00},//R_ObjectSizeStartTh[9:8]                    
	{0x86,0x10},//R_ObjectSizeEndTh[7:0]                      
	{0x87,0x00},//R_ObjectSizeEndTh[9:8]                      
	{0x8B,0x01},//R_Cursor_ObjectSizeTh[7:0]                  
	{0x8D,0x00},//R_TimeDelayNum[7:0]                         
	{0x90,0x06},//R_NoMotionCountThd[6:0]                     
	{0x91,0x06},//R_NoObjectCountThd[6:0]                     
	{0x93,0x0D},//R_XDirectionThd[4:0]                        
	{0x94,0x0A},//R_YDirectionThd[4:0]                        
	{0x95,0x0A},//R_ZDirectionThd[4:0]                        
	{0x96,0x0C},//R_ZDirectionXYThd[4:0]                      
	{0x97,0x05},//R_ZDirectionAngleThd[3:0]                   
	{0x9A,0x14},//R_RotateXYThd[4:0]                          
	{0x9C,0x7F},//Filter setting                              
	{0x9F,0xF9},//R_UseBGModel is enable                      
	{0xA0,0x48},//R_BGUpdateMaxIntensity[7:0]                 
	{0xA5,0x19},//R_FilterAverage_Mode                        
	{0xCC,0x19},//R_YtoZSum[5:0]                              
	{0xCD,0x0B},//R_YtoZFactor[5:0]                           
	{0xCE,0x13},//bit[2:0] = R_PositionFilterLength[2:0]      
	{0xCF,0x62},//bit[3:0] = R_WaveCountThd[3:0]              
	{0xD0,0x21},//R_AbortXYRatio[4:0] & R_AbortLength[6:0]    
	{0xEF,0x01},                                              
	{0x00,0x1E},//Cmd_HSize[5:0]                              
	{0x01,0x1E},//Cmd_VSize[5:0]                              
	{0x02,0x0F},//Cmd_HStart[5:0]                             
	{0x03,0x0F},//Cmd_VStart[5:0]                             
	{0x04,0x02},//Sensor skip & flip                          
	{0x25,0x01},//R_LensShadingComp_EnH                       
	{0x26,0x00},//R_OffsetX[6:0]                              
	{0x27,0x39},//R_OffsetY[6:0]                              
	{0x28,0x7F},//R_LSC[6:0]                                  
	{0x29,0x08},//R_LSFT[3:0]                                 
	{0x30,0x03},//R_LED_SoftStart_time[7:0]                   
	{0x32,0x1A},//R_LED1_DAC_UB[4:0]                          
	{0x33,0x1A},//R_LED2_DAC_UB[4:0]                          
	{0x3E,0xFF},//Cmd_DebugPattern[7:0]                       
	{0x5E,0x3D},//analog voltage setting                      
	{0x65,0xAC},//R_IDLE_TIME[7:0] - 120fps  
	{0x66,0x00},//R_IDLE_TIME[15:8]                           
	{0x67,0x97},//R_IDLE_TIME_SLEEP_1[7:0]                    
	{0x68,0x01},//R_IDLE_TIME_SLEEP_1[15:8]                   
	{0x69,0xCD},//R_IDLE_TIME_SLEEP_2[7:0]                    
	{0x6A,0x01},//R_IDLE_TIME_SLEEP_2[15:8]                   
	{0x6B,0xB0},//R_Obj_TIME_1[7:0]                           
	{0x6C,0x04},//R_Obj_TIME_1[15:8]                          
	{0x6D,0x2C},//R_Obj_TIME_2[7:0]                           
	{0x6E,0x01},//R_Obj_TIME_2[15:8]                          
	{0x72,0x01},//R_TG_EnH                                    
	{0x73,0x35},//Auto Sleep & Wakeup mode                    
	{0x74,0x00},//R_Control_Mode[2:0]                         
	{0x77,0x01},//R_SRAM_Read_EnH   
	{0xEF,0x00},
};
#endif


#ifdef PAJ7620U2_25cm
//Boy modify @ 2015_0923
unsigned char init_register_array[][2] = {	// Initial Gesture
	
	{0xEF,0x00},                                              
	{0x41,0xFF},//R_Int_1_En[7:0]                             
	{0x42,0x01},//R_Int_2_En[7:0]                             
	{0x46,0x2D},//R_AELedOff_UB[7:0]                          
	{0x47,0x0F},//R_AELedOff_LB[7:0]                          
	{0x48,0x80},//R_AE_Exposure_UB[7:0]                       
	{0x49,0x01},//R_AE_Exposure_UB[15:8]                      
	{0x4A,0xC0},//R_AE_Exposure_LB[7:0]                       
	{0x4B,0x00},//R_AE_Exposure_LB[15:8]                      
	{0x4C,0x20},//R_AE_Gain_UB[7:0]                           
	{0x4D,0x00},//R_AE_Gain_LB[7:0]                           
	{0x51,0x10},//R_AE_EnH = 1                                
	{0x5C,0x02},//R_SenClkPrd[5:0]                            
	{0x5E,0x10},//R_SRAM_CLK_manual                           
	{0x80,0x42},//GPIO setting                                
	{0x81,0x44},//GPIO setting                                
	{0x82,0x0C},//Interrupt IO setting                        
	{0x83,0x20},//R_LightThd[7:0]                             
	{0x84,0x30},//R_ObjectSizeStartTh[7:0]                    
	{0x85,0x00},//R_ObjectSizeStartTh[9:8]                    
	{0x86,0x10},//R_ObjectSizeEndTh[7:0]                      
	{0x87,0x00},//R_ObjectSizeEndTh[9:8]                      
	{0x8B,0x01},//R_Cursor_ObjectSizeTh[7:0]                  
	{0x8D,0x00},//R_TimeDelayNum[7:0]                         
	{0x90,0x06},//R_NoMotionCountThd[6:0]                     
	{0x91,0x06},//R_NoObjectCountThd[6:0]                     
	{0x93,0x0D},//R_XDirectionThd[4:0]                        
	{0x94,0x0A},//R_YDirectionThd[4:0]                        
	{0x95,0x0A},//R_ZDirectionThd[4:0]                        
	{0x96,0x0C},//R_ZDirectionXYThd[4:0]                      
	{0x97,0x05},//R_ZDirectionAngleThd[3:0]                   
	{0x9A,0x14},//R_RotateXYThd[4:0]                          
	{0x9C,0x7F},//Filter setting                              
	{0x9F,0xF9},//R_UseBGModel is enable                      
	{0xA0,0x48},//R_BGUpdateMaxIntensity[7:0]                 
	{0xA5,0x19},//R_FilterAverage_Mode                        
	{0xCC,0x19},//R_YtoZSum[5:0]                              
	{0xCD,0x0B},//R_YtoZFactor[5:0]                           
	{0xCE,0x13},//bit[2:0] = R_PositionFilterLength[2:0]      
	{0xCF,0x62},//bit[3:0] = R_WaveCountThd[3:0]              
	{0xD0,0x21},//R_AbortXYRatio[4:0] & R_AbortLength[6:0]    
	{0xEF,0x01},                                              
	{0x00,0x1E},//Cmd_HSize[5:0]                              
	{0x01,0x1E},//Cmd_VSize[5:0]                              
	{0x02,0x0F},//Cmd_HStart[5:0]                             
	{0x03,0x0F},//Cmd_VStart[5:0]                             
	{0x04,0x02},//Sensor skip & flip                          
	{0x25,0x01},//R_LensShadingComp_EnH                       
	{0x26,0x00},//R_OffsetX[6:0]                              
	{0x27,0x39},//R_OffsetY[6:0]                              
	{0x28,0x7F},//R_LSC[6:0]                                  
	{0x29,0x08},//R_LSFT[3:0]                                 
	{0x30,0x03},//R_LED_SoftStart_time[7:0]                   
	{0x32,0x1A},//R_LED1_DAC_UB[4:0]                          
	{0x33,0x1A},//R_LED2_DAC_UB[4:0]                          
	{0x3E,0xFF},//Cmd_DebugPattern[7:0]                       
	{0x5E,0x3D},//analog voltage setting                      
	{0x65,0xAC},//R_IDLE_TIME[7:0] - 120fps  
	{0x66,0x00},//R_IDLE_TIME[15:8]                           
	{0x67,0x97},//R_IDLE_TIME_SLEEP_1[7:0]                    
	{0x68,0x01},//R_IDLE_TIME_SLEEP_1[15:8]                   
	{0x69,0xCD},//R_IDLE_TIME_SLEEP_2[7:0]                    
	{0x6A,0x01},//R_IDLE_TIME_SLEEP_2[15:8]                   
	{0x6B,0xB0},//R_Obj_TIME_1[7:0]                           
	{0x6C,0x04},//R_Obj_TIME_1[15:8]                          
	{0x6D,0x2C},//R_Obj_TIME_2[7:0]                           
	{0x6E,0x01},//R_Obj_TIME_2[15:8]                          
	{0x72,0x01},//R_TG_EnH                                    
	{0x73,0x35},//Auto Sleep & Wakeup mode                    
	{0x74,0x00},//R_Control_Mode[2:0]                         
	{0x77,0x01},//R_SRAM_Read_EnH
	{0xEF,0x00},   
};
#endif

#define INIT_REG_ARRAY_SIZE (sizeof(init_register_array)/sizeof(init_register_array[0]))

