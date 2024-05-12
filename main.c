#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/vreg.h"

const uint32_t PWM_PIN = 14;
const uint32_t PWM_WRAP = 255;
const uint32_t DELAY = 5;
const uint32_t CPU_FREQ = 266000;
const enum vreg_voltage CPU_VOLTAGE = VREG_VOLTAGE_1_25;

uint32_t time_ms()
{
    return (time_us_32() / 1000);
}

struct Pwm
{
    uint slice;
    uint channel;
};
typedef struct Pwm Pwm;

Pwm setup_pwm(uint wrap)
{
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    gpio_set_function(PWM_PIN + 1, GPIO_FUNC_PWM);

    Pwm pwm = {
        pwm_gpio_to_slice_num(PWM_PIN),
        pwm_gpio_to_channel(PWM_PIN),
    };

    pwm_set_wrap(pwm.slice, wrap);
    pwm_set_both_levels(pwm.slice, 0, 0);
    pwm_set_enabled(pwm.slice, true);

    pwm_set_output_polarity(pwm.slice, false, true);

    return pwm;
}

void set_level(Pwm pwm, uint level)
{
    pwm_set_both_levels(pwm.slice, level, level);
}

uint8_t brightness_to_level[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 23, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38, 38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 97, 98, 99, 100, 102, 103, 104, 105, 107, 108, 109, 111, 112, 113, 115, 116, 117, 119, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137, 139, 140, 142, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166, 168, 170, 171, 173, 175, 176, 178, 180, 181, 183, 185, 186, 188, 190, 192, 193, 195, 197, 199, 200, 202, 204, 206, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224, 226, 228, 230, 232, 233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255};
void set_brightness(Pwm pwm, uint brigthness)
{
    set_level(pwm, brightness_to_level[brigthness]);
}

int main()
{
    vreg_set_voltage(CPU_VOLTAGE);
    set_sys_clock_khz(CPU_FREQ, true);

    stdio_init_all();

    Pwm pwm = setup_pwm(PWM_WRAP);

    printf("Booted.\n");
    getchar();
    printf("Hello!\n");

    // measure PWM freq
    uint64_t cycles = 0;
    uint64_t lastCount = 0;
    uint64_t incSum = 0;
    uint64_t incCount = 0;
    uint64_t startTime = time_us_64();
    const uint64_t CYCLE_COUNT = 1e6;
    while (true)
    {
        uint currentCount = pwm_get_counter(pwm.slice);
        if (currentCount >= lastCount)
        {
            incSum += currentCount - lastCount;
            incCount += 1;
            lastCount = currentCount;
        }
        else
        {
            cycles += 1;
            lastCount = currentCount;

            if (cycles > CYCLE_COUNT)
            {
                break;
            }
        }
    }
    uint64_t endTime = time_us_64();
    float duration_us = ((float)(endTime - startTime));
    printf("Finished a million PWM cycles in %.2f s.\n", duration_us * 1.e-6);
    printf("Average number of PWM increments per loop run: %.0f\n", ((float)incSum) / ((float)incCount));

    float cycleFrequency_kHz = (float)CYCLE_COUNT / (duration_us * 1.e-6 * 1e3);
    printf("Cycle frequency: %.2f kHz\n", cycleFrequency_kHz);
    printf("Count frequency: %.2f MHz\n", (float)PWM_WRAP * cycleFrequency_kHz / 1000.);

    while (true)
    {
        for (uint i = 0; i < PWM_WRAP; i++)
        {
            set_brightness(pwm, i);
            sleep_ms(DELAY);
        }
        for (uint i = PWM_WRAP - 1; i > 0; i--)
        {
            set_brightness(pwm, i);
            sleep_ms(DELAY);
        }
    }
}
