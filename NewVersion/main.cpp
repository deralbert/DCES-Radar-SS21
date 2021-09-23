/**
    @file: main.c

    @brief: This application runs on demo Position2Go board with BGT24MTR12 and
   XMC4700 MCU. It consists on radar RangeDoppler demonstration application.
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

#include "gpio_extensions.h"
#include "application.h"

/*
==============================================================================
   2. MAIN METHOD
==============================================================================
 */

DIGITAL_IO_t const *const vibration_engine_1 = &PWM1;
DIGITAL_IO_t const *const vibration_engine_2 = &OUT1;
DIGITAL_IO_t const *const vibration_engine_3 = &OUT2;
DIGITAL_IO_t const *const vibration_engine_4 = &PWM2;

static void delay(uint32_t cycles) {
    for (unsigned int i = 0; i < cycles; ++i) {
        __NOP();
    }
}

static void pwm(unsigned int BitTrigger,
                const DIGITAL_IO_t PortX) { // BitTrigger needs to be within int
                                            // 0(0%) and 255(100%), PortX
    // ist the Port of the Pin which will be manipulated
    for (unsigned int i = 0; i < 256; ++i) {
        DIGITAL_IO_SetOutputHigh(&PortX);
        if (i >= BitTrigger) {
            DIGITAL_IO_SetOutputLow(&PortX);
        }
    }
}

void activate_vibration_engine(DIGITAL_IO_t const *const motorPin, float currentAzimuth,
                  float azimuthLeftThreshold, float azimuthRightThreshold)
{
    if (azimuthLeftThreshold <= currentAzimuth &&
        currentAzimuth < azimuthRightThreshold) {
        DIGITAL_IO_SetOutputHigh(motorPin);
    }
}

void deactivate_all_vibration_engines()
{
	DIGITAL_IO_SetOutputLow(vibration_engine_1);
	DIGITAL_IO_SetOutputLow(vibration_engine_2);
	DIGITAL_IO_SetOutputLow(vibration_engine_3);
	DIGITAL_IO_SetOutputLow(vibration_engine_4);
}

void test_vibration_engines()
{
	static const uint32_t delayTime = 15000000;

	// test from left to right
	for (unsigned int i = 0; i < 2; ++i)
	{
		deactivate_all_vibration_engines();
		DIGITAL_IO_SetOutputHigh(vibration_engine_1);

		delay(delayTime);

		deactivate_all_vibration_engines();
		DIGITAL_IO_SetOutputHigh(vibration_engine_2);

		delay(delayTime);

		deactivate_all_vibration_engines();
		DIGITAL_IO_SetOutputHigh(vibration_engine_3);

		delay(delayTime);

		deactivate_all_vibration_engines();
		DIGITAL_IO_SetOutputHigh(vibration_engine_4);

		delay(delayTime);
	}

	// test all on
	DIGITAL_IO_SetOutputHigh(vibration_engine_1);
	DIGITAL_IO_SetOutputHigh(vibration_engine_2);
	DIGITAL_IO_SetOutputHigh(vibration_engine_3);
	DIGITAL_IO_SetOutputHigh(vibration_engine_4);
	delay(delayTime);
	deactivate_all_vibration_engines();
}

int main(void) {
    DAVE_STATUS_t status;

    // Variables for target_info function
    extern Radar_Handle_t h_radar_device;
    Target_Info_t target_info[MAX_NUM_OF_TARGETS];
    uint8_t num_targets;

    /* Initialize DAVE APPs */
    status = DAVE_Init();
    if (status != DAVE_STATUS_SUCCESS) {
        /* Placeholder for error handler code.
         * The while loop below can be replaced with an user error handler. */
        XMC_DEBUG("DAVE APPs initialization failed\n");
        while (1U)
            ;
    }

    /* Register algorithm processing function:
     * Set the algorithm processing function pointer, it will
     * be used by the application for algorithm data processing */
    app_register_algo_process(range_doppler_do);
    /* Initialize the application */
    app_init();

    test_vibration_engines();

    // experimentally determined value
    // approximately 2 sec = 1350 iterations
    static const uint32_t maxIterations = 1350;
    uint32_t iterationsSinceLastTargetRecognition = 0;

    while (1U) {
        /* Main application process */
        app_process();

        uint16_t error_code = radar_get_target_info(h_radar_device, target_info, &num_targets);

        // deactivate old target after time
        // keep in mind that this spins further after first deactivation
        if (iterationsSinceLastTargetRecognition >= maxIterations)
        {
        	deactivate_all_vibration_engines();
        	iterationsSinceLastTargetRecognition = 0;
        }
        ++iterationsSinceLastTargetRecognition;

        if (error_code != RADAR_ERR_OK) {
            continue;
        }

        // current logic don't work for more than 1 possible target because at the beginning all vibration_engines are deactivated
        // time based filter could improve experience a lot.
        for (int i = 0; i < num_targets; ++i) {
        	Target_Info_t *currentTarget = &target_info[i];

            // skip if distance is too big
            // unit is cm according to datastore.h
            if (currentTarget->radius > 300.00) {
                continue;
            }

            // new valid target found: invalidate vibration engine activation of previous ones
            deactivate_all_vibration_engines();
            // reset target expiration
            iterationsSinceLastTargetRecognition = 0;

            float currentAzimuth = currentTarget->azimuth;

            activate_vibration_engine(vibration_engine_1, currentAzimuth, -40.00, -28.57);

            activate_vibration_engine(vibration_engine_1, currentAzimuth, -28.57, -17.14);
            activate_vibration_engine(vibration_engine_2, currentAzimuth, -28.57, -17.14);

            activate_vibration_engine(vibration_engine_2, currentAzimuth, -17.14, -5.71);

            activate_vibration_engine(vibration_engine_2, currentAzimuth, -5.71, 5.71);
            activate_vibration_engine(vibration_engine_3, currentAzimuth, -5.71, 5.71);

            activate_vibration_engine(vibration_engine_3, currentAzimuth, 5.71, 17.14);

            activate_vibration_engine(vibration_engine_3, currentAzimuth, 17.14, 28.57);
            activate_vibration_engine(vibration_engine_4, currentAzimuth, 17.14, 28.57);

            activate_vibration_engine(vibration_engine_4, currentAzimuth, 28.57, 40.00);
        }
    }
}

/*
==============================================================================
   3. USER CALLBACK FUNCTIONS
==============================================================================
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// add user callback functions here
// they are defined in Application/aplication.c

void algo_completed_cb(void)
{
//
}

void acq_completed_cb(void)
{
//
}

}

/* --- End of File -------------------------------------------------------- */