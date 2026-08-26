#include "HAL.h"
#include "hal_dev.h"

static void* EncoderPush = NULL;

static bool EncoderEnable = true;
static volatile int32_t EncoderDiff = 0;
static bool EncoderDiffDisable = false;

static void Buzz_Handler(int dir)
{
    static const uint16_t freqStart = 2000;
    static uint16_t freq = 2000;
    static uint32_t lastRotateTime;

    if(millis() - lastRotateTime > 1000)
    {
        freq = freqStart;
    }
    else
    {
        if(dir > 0)
        {
            freq += 100;
        }

        if(dir < 0)
        {
            freq -= 100;
        }

        freq = constrain(freq, 100, 20 * 1000);
    }

    lastRotateTime = millis();
    Buzz_Tone(freq, 5);
}

static void Encoder_EventHandler(void)
{
    if(!EncoderEnable || EncoderDiffDisable)
    {
        return;
    }

    int dir = (digitalRead(CONFIG_ENCODER_B_PIN) == LOW ? -1 : +1);
    EncoderDiff += dir;
    Buzz_Handler(dir);
}

static void Encoder_PushHandler(void* btn, int event)
{
    (void)btn;
    if(event == DEV_BTN_EVENT_PRESSED)
    {
        EncoderDiffDisable = true;
    }
    else if(event == DEV_BTN_EVENT_RELEASED)
    {
        EncoderDiffDisable = false;
    }
    else if(event == DEV_BTN_EVENT_LONG_PRESSED)
    {
        Power_Shutdown();
        Audio_PlayMusic("Shutdown");
    }
}

void Encoder_Init(void)
{
    pinMode(CONFIG_ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(CONFIG_ENCODER_B_PIN, INPUT_PULLUP);
    pinMode(CONFIG_ENCODER_PUSH_PIN, INPUT_PULLUP);

    attachInterrupt(CONFIG_ENCODER_A_PIN, Encoder_EventHandler, FALLING);

    EncoderPush = DevBtn_Create(CONFIG_POWER_SHUTDOWM_DELAY);
    DevBtn_EventAttach(EncoderPush, Encoder_PushHandler);
}

void Encoder_Update(void)
{
    DevBtn_EventMonitor(EncoderPush, Encoder_GetIsPush());
}

int32_t Encoder_GetDiff(void)
{
    int32_t diff = EncoderDiff;
    EncoderDiff = 0;
    return diff;
}

bool Encoder_GetIsPush(void)
{
    if(!EncoderEnable)
    {
        return false;
    }

    return (digitalRead(CONFIG_ENCODER_PUSH_PIN) == LOW);
}

void Encoder_SetEnable(bool en)
{
    EncoderEnable = en;
}
