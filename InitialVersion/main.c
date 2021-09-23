**
    @file: main.c
    @brief: This application runs on demo Position2Go board with BGT24MTR12 and XMC4700 MCU.
            It consists on radar RangeDoppler demonstration application.
*/

/* ===========================================================================
** Copyright (C) 2018-2019 Infineon Technologies AG
** All rights reserved.
** ===========================================================================
**
** ===========================================================================
** This document contains proprietary information of Infineon Technologies AG.
** Passing on and copying of this document, and communication of its contents
** is not permitted without Infineon's prior written authorization.
** ===========================================================================
*/

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
 */

#include "application.h"

/*
==============================================================================
   2. MAIN METHOD
==============================================================================
 */


static void delay(uint32_t cycles){
	for(unsigned int i = 0; i < cycles; ++i)
	{
	__NOP();
	}
}


static void pwm(int BitTrigger, const DIGITAL_IO_t PortX){ // BitTrigger needs to be within int 0(0%) and 255(100%), PortX ist the Port of the Pin which will be manipulated
		for(unsigned int i = 0; i < 256; ++i){
		DIGITAL_IO_SetOutputHigh(&PortX);
			if(i >= BitTrigger){
				DIGITAL_IO_SetOutputLow(&PortX);
			}
		}
}



//_________________________________________________________________________________


//________________________________________________________________________________________
//void algo_completed_cb(void);

int main(void)
{
  DAVE_STATUS_t status;


  //Variables for target_info function
  extern Radar_Handle_t h_radar_device;
  Target_Info_t target_info[MAX_NUM_OF_TARGETS];
  uint8_t num_targets;

  /* Initialize DAVE APPs */
  status = DAVE_Init();
  uint8_t error_code;
  if(status != DAVE_STATUS_SUCCESS)
  {
    /* Placeholder for error handler code.
     * The while loop below can be replaced with an user error handler. */
    XMC_DEBUG("DAVE APPs initialization failed\n");
    while(1U);
  }
  
  /* Register algorithm processing function:
   * Set the algorithm processing function pointer, it will
   * be used by the application for algorithm data processing */
  app_register_algo_process(range_doppler_do);
  //DIGITAL_IO_SetOutputHigh(&PWM2);
  /* Initialize the application */
  app_init();
  /* Infinite loop */
  while(1U)
  {
    /* Main application process */
    app_process();
    //DIGITAL_IO_SetOutputHigh(&PWM1);
    //DIGITAL_IO_SetOutputHigh(&PWM2);

    if((error_code = radar_get_target_info(h_radar_device, target_info, &num_targets)) == RADAR_ERR_OK){
		 // ...iterate through all targets...
		   for(int i= 0; i< num_targets; i++){
		    //check for angle cases while FOV of chip is 77 degree
		    	// case 1: [0<angle<40] /degree
		    	if(target_info[i].azimuth >= 0 && target_info[i].azimuth < 40 ){
		    		DIGITAL_IO_SetOutputHigh(&PWM1);
		    	}else{
		    		DIGITAL_IO_SetOutputLow(&PWM1);
		    	}
		    	// case 2: [-40<angle<0] /degree
		    	if(target_info[i].azimuth > -40 && target_info[i].azimuth < 0 ){
		    		DIGITAL_IO_SetOutputHigh(&PWM2);
		    	}else{
		    		DIGITAL_IO_SetOutputLow(&PWM2);
		    	}
		   	}


    }
  }
}

/*
==============================================================================
   3. USER CALLBACK FUNCTIONS
==============================================================================
 */

void acq_completed_cb(void)
{
 /*
  * The following code shows an example of how to access raw data buffer.
  */

  /*
  acq_buf_obj *p_acq_buf 	= ds_get_active_acq_buf();
  uint8_t *raw_data 		= p_acq_buf->p_acq_buf;
  uint32_t raw_data_size 	= p_acq_buf->used_size_of_acq_buffer;
  
  -- Add your code here --
  
  */
}

void algo_completed_cb(void)
{
 /*
  * The following sample code could be seen as a small example,
  * how to get the information from the algorithm and use it accordingly.
  */
  
/*
  static float s_max_level = 0;
  static uint32_t countFrames = 0;
  static uint32_t s_max_num_targets = 0;
  
  extern Radar_Handle_t h_radar_device;
  
  Target_Info_t target_info[MAX_NUM_OF_TARGETS];
  
  uint8_t num_targets;
  
  if(radar_get_target_info(h_radar_device, target_info, &num_targets) == RADAR_ERR_OK)
  {
    float max_level = 0;
    for(int i= 0; i< num_targets; i++)
    {
      if(target_info[i].level > max_level)
      {
        s_max_level = target_info[i].level;
      }
    }
    countFrames += 1;
    
    if(num_targets > s_max_num_targets)
    {
      s_max_num_targets = num_targets;
    }
    
  }
  return */
}

/* --- End of File -------------------------------------------------------- */