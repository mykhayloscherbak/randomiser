#include "unity.h"
#include "Bll.h"
#include "fonts.h"
#include "arial72.h"
#include "arial8.h"
#include "mock_Gpio.h"
#include "mock_pwm.h"
#include "mock_Clock.h"
#include "mock_power.h"
#include "mock_uc1701x.h"
#include "mock_buttons.h"
#include "mock_Spi.h"
#include "backlight.h"


void test_bll_call(void)
{
   GetTicksCounter_ExpectAndReturn(5);
   Buttons_Process_Expect(0);
   Gpio_Set_Bit_Ignore();
   SetTimer_Ignore();
   IsTimerPassed_IgnoreAndReturn(0);
   TEST_ASSERT_EQUAL(MainLoop_Iteration(),1);
   GetTicksCounter_ExpectAndReturn(5);
   TEST_ASSERT_EQUAL(MainLoop_Iteration(),0);

}
