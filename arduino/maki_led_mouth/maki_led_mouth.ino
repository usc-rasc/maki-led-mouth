#include <arduino.h>

#include "bit_utils.h"
#include "tlc59xx_driver.h"
#include "tlc59xx_packet.h"

typedef tlc59xx::Driver _Driver;
typedef tlc59xx::Packet _Packet;
typedef tlc59xx::FunctionControlMessage _FunctionControlMsg;

// use pin 4 for data and pin 6 for clock
_Driver led_driver( 4, 6 );

void setup()
{
//	Serial.begin( 115200 );
	// keep inputs from floating and declare pin use case
	led_driver.initialize();

	_Packet packet;

	// reset function flags
	packet.function_control_msg_.setGrayscaleClockSource( _FunctionControlMsg::GrayscaleClockSource::INTERNAL_CLOCK );
	packet.function_control_msg_.setOutputTiming( _FunctionControlMsg::OutputTiming::RISING_EDGE );
	packet.function_control_msg_.setTimingResetMode( _FunctionControlMsg::TimingResetMode::RESET_DISABLED );
	packet.function_control_msg_.setDisplayRepeatMode( _FunctionControlMsg::DisplayRepeatMode::REPEAT_DISABLED );
	packet.function_control_msg_.setBlankMode( _FunctionControlMsg::BlankMode::OUTPUTS_DISABLED );

	// zero-out brightness
	packet.global_brightness_control_msg_.setValues( 0, 0, 0 );

	// sero-out colors
	packet.grayscale_control_msg_array_[0].setValues( 0, 0, 0 );
	packet.grayscale_control_msg_array_[1].setValues( 0, 0, 0 );
	packet.grayscale_control_msg_array_[2].setValues( 0, 0, 0 );
	packet.grayscale_control_msg_array_[3].setValues( 0, 0, 0 );

	// send a copy of the packet for each IC (last IC is addressed first)
	for( uint8_t chip_idx = 0; chip_idx < 5; ++chip_idx )
	{
		packet.send( led_driver );
	}

	// delay to trigger latch pulse
	delay( 1 );
}

void loop()
{
//	Serial.println( ">>>>>>>>>>" );
	_Packet packet;

	// set function flags
	// use internal clock for duty cycle
	packet.function_control_msg_.setGrayscaleClockSource( _FunctionControlMsg::GrayscaleClockSource::INTERNAL_CLOCK );
	// turn LEDs on at rising edge of duty cycle clock
	packet.function_control_msg_.setOutputTiming( _FunctionControlMsg::OutputTiming::RISING_EDGE );
	// automatically reset the internal PWM counter (used to simulate analog duty cycle) during a latch pulse
	packet.function_control_msg_.setTimingResetMode( _FunctionControlMsg::TimingResetMode::RESET_ENABLED );
	// automatically repeat the latched data if no updates were made in a given display cycle
	packet.function_control_msg_.setDisplayRepeatMode( _FunctionControlMsg::DisplayRepeatMode::REPEAT_ENABLED );
	// enable current-sinking outputs
	packet.function_control_msg_.setBlankMode( _FunctionControlMsg::BlankMode::OUTPUTS_ENABLED );

	// set current limit for each channel (R, G, B)
	// the values here were selected to ensure the overall current would remain below 900 mA when all LEDs are at 25% duty cycle
	packet.global_brightness_control_msg_.setValues( 75, 75, 75 );

	// for each IC
	// display a color pallete
	// keep in mind that the 0th chip is the nth chip in the data path (farthest from the data/clock source)
	// and the nth chip is the first chip in the data path
	for( uint8_t chip_idx = 0; chip_idx < 5; ++chip_idx )
	{
		if( chip_idx == 4 )
		{
			// set the RGB values for each output group [0-4] on the current IC
			packet.grayscale_control_msg_array_[0].setValuesPercent( 0.25f, 0.25f, 0.25f );
			packet.grayscale_control_msg_array_[1].setValuesPercent( 0.25f, 0.0f, 0.0f );
			packet.grayscale_control_msg_array_[2].setValuesPercent( 0.0f, 0.25f, 0.0f );
			packet.grayscale_control_msg_array_[3].setValuesPercent( 0.0f, 0.0f, 0.25f );
		}
		else if( chip_idx == 3 )
		{
			// set the RGB values for each output group [0-4] on the current IC
			packet.grayscale_control_msg_array_[0].setValuesPercent( 0.25f, 0.25f, 0.0f );
			packet.grayscale_control_msg_array_[1].setValuesPercent( 0.0f, 0.0, 0.0 );
			packet.grayscale_control_msg_array_[2].setValuesPercent( 0.0f, 0.25f, 0.25f );
			packet.grayscale_control_msg_array_[3].setValuesPercent( 0.25f, 0.0f, 0.25f );
		}
		else if( chip_idx == 2 )
		{
			// set the RGB values for each output group [0-4] on the current IC
			packet.grayscale_control_msg_array_[0].setValuesPercent( 0.25f, 0.125f, 0.0f );
			packet.grayscale_control_msg_array_[1].setValuesPercent( 0.125f, 0.25f, 0.0f );
			packet.grayscale_control_msg_array_[2].setValuesPercent( 0.0f, 0.25f, 0.125f );
			packet.grayscale_control_msg_array_[3].setValuesPercent( 0.0f, 0.125f, 0.25f );
		}
		else if( chip_idx == 1 )
		{
			// set the RGB values for each output group [0-4] on the current IC
			packet.grayscale_control_msg_array_[0].setValuesPercent( 0.25f, 0.0f, 0.125f );
			packet.grayscale_control_msg_array_[1].setValuesPercent( 0.125f, 0.0f, 0.25f );
			packet.grayscale_control_msg_array_[2].setValuesPercent( 0.9f, 0.9f, 0.9f );
			packet.grayscale_control_msg_array_[3].setValuesPercent( 0.7f, 0.7f, 0.7f );
		}
		else // chip_idx == 0
		{
			// set the RGB values for each output group [0-4] on the current IC
			packet.grayscale_control_msg_array_[0].setValuesPercent( 0.5f, 0.5f, 0.5f );
			packet.grayscale_control_msg_array_[1].setValuesPercent( 0.3f, 0.3f, 0.3f );
			packet.grayscale_control_msg_array_[2].setValuesPercent( 0.0f, 0.0, 0.0 );
			packet.grayscale_control_msg_array_[3].setValuesPercent( 0.1f, 0.1f, 0.1f );
		}

		// toggle the OutputTiming bit on every other IC to avoid noise from nearby ICs
		if( chip_idx % 2 == 0 ) packet.function_control_msg_.setOutputTiming( _FunctionControlMsg::OutputTiming::RISING_EDGE );
		else packet.function_control_msg_.setOutputTiming( _FunctionControlMsg::OutputTiming::FALLING_EDGE );

		// write out the entire packet (need to do this once for each IC)
		packet.send( led_driver );
	}

	// delay to trigger latch pulse
	delay( 1 );
}
