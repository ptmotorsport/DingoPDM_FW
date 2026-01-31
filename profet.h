#pragma once

#include <cstdint>
#include "port.h"
#include "config.h"
#include "enums.h"
#include "pwm.h"

//=============================================================================
// PWM read delay = timer count from PWM going high till ready to read ADC
//=============================================================================
// BTS7002-1EPP or BTS70012-1ESP
// Typical switch on time + current sense settle time = 130us + 5us = 135us, round up to 140us
// NOTE: Section 9.4 of the datasheet says this should be multiplied by 3x
// NOTE: Other datasheets do not say this, not using 3x
// ADC conversion = 20us
#define PWM_READ_DELAY_SINGLE_CH 160
// Min duty cycle @ 100Hz = 160us / 10ms  = 1.6%
// Min duty cycle @ 200Hz = 160us / 5ms   = 3.2%
// Min duty cycle @ 400Hz = 160us / 2.5ms = 6.4%

// BTS7008-2EPA
// Typical switch on time + current sense settle time = 60us + 5us = 65us, round up to 70us
// ADC conversion = 20us
#define PWM_READ_DELAY_DOUBLE_CH 90
// Min duty cycle @ 100Hz = 90us / 10ms  = 0.9%
// Min duty cycle @ 200Hz = 90us / 5ms   = 1.8%
// Min duty cycle @ 400Hz = 90us / 2.5ms = 3.6%
//=============================================================================

class Profet
{
public:
    Profet( int num, ProfetModel model, ioline_t in, ioline_t den, ioline_t dsel, AnalogChannel ain, 
            PWMDriver *pwmDriver, const PWMConfig *pwmCfg, PwmChannel pwmCh)
        :   m_num(num), m_model(model), m_in(in), m_den(den), m_dsel(dsel), m_ain(ain), 
            m_pwmDriver(pwmDriver), m_pwmCfg(pwmCfg), m_pwmChannel(pwmCh),
            pwm(pwmDriver, pwmCfg, pwmCh)
    {
        // Always on
        palSetLine(m_den);

        switch (model)
        {
        case ProfetModel::BTS7002_1EPP:
            fKILIS = BTS7002_1EPP_KILIS;
            nPwmReadDelay = PWM_READ_DELAY_SINGLE_CH;
            break;
        case ProfetModel::BTS7008_2EPA_CH1:
        case ProfetModel::BTS7008_2EPA_CH2:
            fKILIS = BTS7008_2EPA_KILIS;
            nPwmReadDelay = PWM_READ_DELAY_DOUBLE_CH;
            break;
        case ProfetModel::BTS70012_1ESP:
            fKILIS = BTS70012_1ESP_KILIS;
            nPwmReadDelay = PWM_READ_DELAY_SINGLE_CH;
            break;
        }
    }

    void SetConfig(Config_Output *config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];

        pwm.SetConfig(&config->stPwm, pVarMap);

        // Calculate I²t damage thresholds based on config
        // Threshold = (current_limit)² × (multiplier / 10.0)
        // Example: 10A limit, multiplier 50 → threshold = (10)² × 5.0 = 500 A²·s
        float fCurrentLimit = (float)(pConfig->nCurrentLimit) / 10.0f; // Convert from A×10 to A
        float fInrushLimit = (float)(pConfig->nInrushLimit) / 10.0f;
        
        fDamageThreshold = (fCurrentLimit * fCurrentLimit) * ((float)pConfig->nFuseDamageLimit / 10.0f);
        fInrushDamageThreshold = (fInrushLimit * fInrushLimit) * ((float)pConfig->nInrushFuseDamageLimit / 10.0f);
        
        // Initialize damage accumulators
        fDamageAccumulated = 0.0f;
        fInrushDamageAccumulated = 0.0f;
        nLastDamageUpdateTime = SYS_TIME;
    }

    void Update(bool bOutEnabled);

    uint16_t GetCurrent() { return nCurrent; }
    ProfetState GetState() { return eState; }
    uint16_t GetOcCount() { return nOcCount; }
    uint8_t GetDutyCycle()
    {
        if (eState == ProfetState::On)
            return pwm.GetDutyCycle();

        return 0;
    };
    uint8_t GetDamagePercent()
    {
        if (fDamageThreshold <= 0.0f)
            return 0;
        
        uint8_t nPercent = (uint8_t)(254.0f * fDamageAccumulated / fDamageThreshold);
        if (nPercent > 254)
            nPercent = 254;
        
        return nPercent;
    };
    float GetDamageAccumulated() { return fDamageAccumulated; };

    static MsgCmdResult ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx);

    uint16_t nOutput;

private:
    const uint16_t m_num;
    const ProfetModel m_model;
    const ioline_t m_in;
    const ioline_t m_den;
    const ioline_t m_dsel;
    const AnalogChannel m_ain;
    PWMDriver *m_pwmDriver;
    const PWMConfig *m_pwmCfg;
    const PwmChannel m_pwmChannel;

    Config_Output *pConfig;

    uint16_t *pInput;

    ProfetState eState;
    ProfetState eReqState;
    ProfetState eLastState;

    uint16_t nCurrent;    // Scaled current value (amps)
    uint16_t nIS;         // Raw analog current value
    uint16_t nLastIS = 0; // Last analog current value
    float fKILIS;         // Current scaling factor

    bool bInRushActive;
    uint32_t nInRushOnTime;

    uint16_t nOcCount;       // Number of overcurrents
    uint32_t nOcTriggerTime; // Time of overcurrent

    // I²t fuse damage tracking
    float fDamageAccumulated;        // Normal operation damage (A²·s)
    float fDamageThreshold;          // Normal operation trip threshold (A²·s)
    float fInrushDamageAccumulated;  // Inrush damage (A²·s)
    float fInrushDamageThreshold;    // Inrush trip threshold (A²·s)
    uint32_t nLastDamageUpdateTime;  // Last damage calculation time (ms)

    Pwm pwm;
    uint16_t nPwmReadDelay = 0;
};