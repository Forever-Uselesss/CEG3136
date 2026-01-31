/* // Motor controller app
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "motor.h"
#include "display.h"
#include "touchpad.h"
#include "gpio.h"
#include "timer.h"
#include "adc.h"
#include "sysclk.h"
#include "touchpad.h"

// GPIO pins
// Refer to Lab User’s Guide
static const Pin_t AI1 = {GPIOB, 10}; // Pin P??? -> Motor driver
static const Pin_t AI2 = {GPIOB, 11}; // Pin P??? -> Motor driver
static const Pin_t STBY = {GPIOE, 15}; // Pin P??? -> Motor driver
static const Pin_t EncA = {GPIOB, 0}; // Pin P??? <- Rotary encoder A
static const Pin_t EncB = {GPIOB, 1}; // Pin P??? <- Rotary encoder B
// Analog-to-digital converter for potentiometer
// Refer to Lab User’s Guide and MCU datasheet
static const ADCInput_t Pot = {ADC1, 1, {GPIOC, 0}}; // ADC1 Chan1 <- P???
// Timer channels
// Refer to Lab User’s Guide and MCU datasheet
static const TimerIO_t Motor = {TIM1, 1, {GPIOE,9}, 1}; // Timer? Chan? -> P??? AF?
// Timer period
#define PWM_PSC (100 - 1) // Pre-scaler
#define PWM_ARR (240 - 1) // Auto-reload
#define PWM_RCR ( 50 - 1) // Repetition count
#define MAX_SPEED 350.0
static enum { CW=0, CCW=1 } direction = CCW; // Clockwise or counter-clockwise
static enum { OL=0, CL=1, CLT=2 } loopMode = OL; // Open/closed loop (w/ tuning)
static float rpmScalingFactor;
static float desiredRPM = 0;
static float errorRpm;
static float error = 0;
static float errorRpmSum = 0;
static float errorSum = 0;
// PI controller parameters
// Note: Update defaults for Np and Ni after tuning
// Kp = Np * dp
int Np = 5;
float dp = 0.1;
// Ki = Ni * di
int Ni = 5;
float di = 0.001;
uint32_t pulsesA = 0;
uint32_t pulsesB = 0;
uint32_t timerCount = 0;
uint32_t totalPulses = 0;
uint32_t prevTimerCount = 0;
// Interrupt callback functions
static void CallbackMotor(void);
static void CallbackEncA(void);
static void CallbackEncB(void);
// Change motor direction
// Refer to motor driver datasheet
void MotorDirection(int dir) {
 if (dir == CW) {
 // Clockwise
         GPIO_Output(AI1,HIGH);  // AI1 = HIGH
         GPIO_Output(AI2,LOW); // AI2 = LOW
 }
 else {
         GPIO_Output(AI1,LOW);  // AI1 = LOW
         GPIO_Output(AI2,HIGH);  // AI2 = HIGH
 }
}
// Initialize app
void Init_Motor (void) {
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
#endif
 ADC_Enable(Pot);

 // Configure GPIO pins
 // Refer to motor controller datasheet for outputs in stop mode
 GPIO_Enable(AI1);
 GPIO_Mode(AI1,OUTPUT);
 GPIO_Output(AI1,LOW);     // AI1 = HIGH

 GPIO_Enable(AI2);
 GPIO_Mode(AI1,OUTPUT);
 GPIO_Output(AI2,LOW);     // AI1 = HIGH

 GPIO_Enable(STBY);
 GPIO_Mode(STBY,OUTPUT);
 GPIO_Output(STBY,1);    // STBY = HIGH

 GPIO_Enable(EncA);
 GPIO_Mode(EncA,INPUT);

 GPIO_Enable(EncB);
 GPIO_Mode(EncB,INPUT);

 // Register callbacks for rotary encoder inputs
 GPIO_Callback(EncA, CallbackEncA, RISE);
 GPIO_Callback(EncB, CallbackEncB, RISE);


 TimerEnable(Motor);
 TimerPeriod(Motor, PWM_PSC, PWM_ARR, PWM_RCR);
 TimerMode(Motor, OUTCMP, PWM1);
 TimerCallback(Motor, CallbackMotor, UP);
 TimerStart(Motor, OUTCMP);
 MotorDirection(direction);
 // Refer to Lab Manual
 float fup = SYSCLK_FREQ / ((PWM_PSC + 1)*(PWM_ARR + 1)*(PWM_RCR + 1));
 float ppr = 34.0 * 11.0 *2.0;  // gear ratio = 34:1  resolution = 11 pulses
 //number of encoders =2
 rpmScalingFactor = (fup/ppr) *60.0;
}
// Execute app
void Task_Motor (void) {
 float motorDrivePercentage = (float)ADC_Read(Pot) / 4096.0;
 desiredRPM = motorDrivePercentage * MAX_SPEED;
 if (timerCount-prevTimerCount > 0) {
 float measuredRPM = (float)totalPulses*rpmScalingFactor;
 if (loopMode == OL) {
 TimerOutput(Motor, (uint16_t) (motorDrivePercentage * (float)(PWM_ARR + 1)));
 // Display status for Open Loop mode
 DisplayPrint(MOTOR, 0, " OL %3d %%", (int)(motorDrivePercentage*100));
 DisplayPrint(MOTOR, 1, "%s %3d RPM", direction == CW ? " CW" : "CCW", (int)measuredRPM);
 }
 else {
 // Closed Loop control
 float Kp = (float)Np * dp;
 float Ki = (float)Ni * di;
 errorRpm = desiredRPM - measuredRPM;
 errorRpmSum += errorRpm;
 // Scale errorRpm to Timer CCR
 error = errorRpm / MAX_SPEED * (float)(PWM_ARR + 1);
 errorSum = errorRpmSum / MAX_SPEED * (float)(PWM_ARR + 1);
 TimerOutput(Motor,round((Kp*error + Ki*errorSum)));
 if (loopMode == CLT) {
 // Display status for Closed Loop mode /w tuning enabled
         DisplayPrint(MOTOR, 0, "Kp:%2.2f %3d RPM",Kp, (int)(desiredRPM));
         DisplayPrint(MOTOR, 1, "Ki:%2.2f %3d RPM",Ki, (int)(measuredRPM));
 }
 else {
 // Display status for normal Closed Loop mode
         DisplayPrint(MOTOR, 0, " CL %3d RPM",(int)(desiredRPM));
         DisplayPrint(MOTOR, 1, "%s %3d RPM", direction == CW ? " CW" : "CCW", (int)measuredRPM);


 }
 }
 totalPulses = 0;
 prevTimerCount = timerCount;
 }
 // Touchpad controls
 switch (TouchInput(MOTOR)) {
 // Motor direction: update and apply
 case 1:
         direction = CW;
         MotorDirection(direction);
         break;
 case 4:
         direction = CCW;
         MotorDirection(direction);
         break;

 // Controller operating mode
 case 7:
         loopMode = OL;
         break;
 case 8:
         loopMode = CL;
         break;
 case 9:
         loopMode = CLT;
         break;

 // Controller tuning (must be in CLT mode to take effect)
 case 2:
         if(loopMode == CLT) {
                 Np++;
                 float Kp = (float)Np * dp;
                 DisplayPrint(MOTOR, 0, "Kp:%2.2f %3d RPM",Kp, (int)(desiredRPM));


         }
         break;
 case 5:
         if(loopMode == CLT && Np > 0) {
                 Np--;
                 float Kp = (float)Np * dp;
                 DisplayPrint(MOTOR, 0, "Kp:%2.2f %3d RPM",Kp, (int)(desiredRPM));


         }
         break;
 case 3:
         if(loopMode == CLT) {
                 Ni++;

         }
         break;
 case 6:
         if(loopMode == CLT && Ni > 0) {
                 Ni--;
         }
         break;


 // Clear Kp and Ki
 case 0:
         if(loopMode == CLT) {
                 Np =0;
                 Ni =0;
         }
         break;



 default: break;
 }
}



// Timer 1 update
void CallbackMotor (void) {
 timerCount++;
 totalPulses = pulsesA+pulsesB;
 pulsesA = 0;
 pulsesB = 0;
}
// Rotary encoder A rising edge
void CallbackEncA (void) {
 pulsesA++;
}
// Rotary encoder B rising edge
void CallbackEncB (void) {
 pulsesB++;
}



*/

// Motor controller app

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "display.h"
#include "gpio.h"
#include "motor.h"
#include "sysclk.h"
#include "timer.h"
#include "touchpad.h"

// GPIO pins
// Refer to Lab User’s Guide
static const Pin_t AI1 = {GPIOB, 10};  // Channel A1, port B, pin 10 // Pin P??? -> Motor driver
static const Pin_t AI2 = {GPIOB, 11};  // Channel A2, port B, pin 11 // Pin P??? -> Motor driver
static const Pin_t STBY = {GPIOE, 15}; // Standby Control signal, port E, pin 15 // Pin P??? -> Motor driver
static const Pin_t EncA = {GPIOB, 0};  // Pin PB0 <- Rotary encoder A
static const Pin_t EncB = {GPIOB, 1};  // Pin PB1 <- Rotary encoder B

// Analog-to-digital converter for potentiometer
// Refer to Lab User’s Guide
static const ADCInput_t Pot = {ADC1, 1, {GPIOC, 0}}; // ADC1 Chan1 <- P???
//********not sure if that's right^  ^       ^        (pg. 37)

// Timer channels
// Refer to Lab User’s Guide
static const TimerIO_t Motor = {TIM1, 1, {GPIOE, 9}, 1}; // Timer1 Chan1 -> PE9 AF1

// Timer period
#define PWM_PSC (100 - 1) // Pre-scaler
#define PWM_ARR (240 - 1) // Auto-reload
#define PWM_RCR (50 - 1)  // Repetition count

#define MAX_SPEED 360.0
static enum { CW = 0, CCW = 1 } direction = CCW;       // Clockwise or counter-clockwise
static enum { OL = 0, CL = 1, CLT = 2 } loopMode = OL; // Open/closed loop (w/ tuning)

static float rpmScalingFactor;
static float desiredRPM = 0;
static float errorRpm;
static float error = 0;
static float errorRpmSum = 0;
static float errorSum = 0;

// PI controller parameters
// Note: Update defaults for Np and Ni after tuning
// Kp = Np * dp
int Np = 5;
float dp = 0.1;
// Ki = Ni * di
int Ni = 5;
float di = 0.001;

uint32_t pulsesA = 0;
uint32_t pulsesB = 0;
uint32_t timerCount = 0;
uint32_t totalPulses = 0;
uint32_t prevTimerCount = 0;

// Interrupt callback functions
static void CallbackMotor(void);
static void CallbackEncA(void);
static void CallbackEncB(void);

// Change motor direction
// Refer to motor driver datasheet
void MotorDirection(int dir) {
  if (dir == CW) {
    // Clockwise
    GPIO_Output(AI1, LOW);  // AI1 = HIGH
    GPIO_Output(AI2, HIGH); // AI2 = LOW
  } else {
    GPIO_Output(AI1, HIGH); // AI1 = LOW
    GPIO_Output(AI2, LOW);  // AI2 = HIGH
  }
}

// Initialize app
void Init_Motor(void) {

#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
#endif

  ADC_Enable(Pot); // Speed control potentiometer

  // Configure GPIO pins
  // Refer to motor controller datasheet for outputs in stop mode
  // Register callbacks for rotary encoder inputs

  GPIO_Enable(AI1);
  GPIO_Mode(AI1, OUTPUT);
  GPIO_Output(AI1, LOW);

  GPIO_Enable(AI2);
  GPIO_Mode(AI2, OUTPUT);
  GPIO_Output(AI2, LOW);

  GPIO_Enable(STBY);
  GPIO_Mode(STBY, OUTPUT);
  GPIO_Output(STBY, HIGH);

  GPIO_Enable(EncA);
  GPIO_Mode(EncA, INPUT);
  GPIO_Callback(EncA, CallbackEncA, RISE);

  GPIO_Enable(EncB);
  GPIO_Mode(EncB, INPUT);
  GPIO_Callback(EncB, CallbackEncB, RISE);

  TimerEnable(Motor);
  TimerPeriod(Motor, PWM_PSC, PWM_ARR, PWM_RCR);
  TimerMode(Motor, OUTCMP, PWM1);
  TimerCallback(Motor, CallbackMotor, UP);
  TimerStart(Motor, OUTCMP);

  MotorDirection(direction);

  // rpmScalingFactor = 1/(2*374);
  float freqUpdate = SYSCLK_FREQ / ((PWM_PSC + 1) * (PWM_ARR + 1) * (PWM_RCR + 1));
  float PulsesPerRev = 11.0 * 34.0 * 2.0; // 748 pulses per revolution
  rpmScalingFactor = (freqUpdate / PulsesPerRev) * 60.0;
}

// Execute app
void Task_Motor(void) {

  float motorDrivePercentage = (float)ADC_Read(Pot) / 4096.0;
  desiredRPM = motorDrivePercentage * MAX_SPEED;

  if (timerCount - prevTimerCount > 0) {
    float measuredRPM = (float)totalPulses * rpmScalingFactor;

    if (loopMode == OL) {
      TimerOutput(Motor, (uint16_t)(motorDrivePercentage * (float)(PWM_ARR + 1)));

      // Display status for Open Loop mode
      DisplayPrint(MOTOR, 0, " OL     %3d %%", (int)(motorDrivePercentage * 100));
      DisplayPrint(MOTOR, 1, "%cCW     %3d RPM", direction == CCW ? 'C' : ' ', (int)measuredRPM);

    }

    else {
      // Closed Loop control

      float Kp = (float)Np * dp;
      float Ki = (float)Ni * di;

      errorRpm = desiredRPM - measuredRPM;
      errorRpmSum += errorRpm;

      // Convert errorRpm to change on TIM1->CCR1
      error = errorRpm / MAX_SPEED * (float)(PWM_ARR + 1);
      errorSum = errorRpmSum / MAX_SPEED * (float)(PWM_ARR + 1);

      TimerOutput(Motor, round((Kp * error + Ki * errorSum)));

      if (loopMode == CLT) {
        // Display status for Closed Loop mode /w tuning enabled
        DisplayPrint(MOTOR, 0, "Kp:%.3f %3d RPM", Kp, (int)(desiredRPM));
        DisplayPrint(MOTOR, 1, "Ki:%.3f %3d RPM", Ki, (int)(measuredRPM));

      } else {
        // Display status for normal Closed Loop mode
        /* DisplayPrint(MOTOR, 0, "CL, SetRPM:%d", (int)desiredRPM);
         if (direction == CW)
                 DisplayPrint(MOTOR, 1, "CW, RPM:%d", (int)measuredRPM);
         else if(direction == CCW)
                 DisplayPrint(MOTOR, 1, "CCW, RPM:%d", (int)measuredRPM);*/

        DisplayPrint(MOTOR, 0, " CL %3d RPM", (int)(desiredRPM));
        DisplayPrint(MOTOR, 1, "%s %3d RPM", direction == CW ? " CW" : "CCW", (int)measuredRPM);
      }
    }

    totalPulses = 0;
    prevTimerCount = timerCount;
  }

  // Touchpad controls
  switch (TouchInput(MOTOR)) {

  // Motor direction: update and apply
  case 1:
    direction = CW; // button 1 makes motor go CW
    MotorDirection(direction);
    break;

  case 4:
    direction = CCW; // button 4 makes motor go CCW
    MotorDirection(direction);
    break;

    // Controller operating mode
  case 7:
    loopMode = OL; // Operating mode set to open loop
    break;

  case 8:
    loopMode = CL; // Operating mode set to closed loop
    break;

  case 9:
    loopMode = CLT; // Operating mode set to closed loop tuning
    break;

    if (loopMode == CLT) { // Controller tuning (must be in CLT mode to take effect)

    case 2: // Incrementing Kp
      Np++;
      break;

    case 5: // Decrementing Kp
      if (Np == 0)
        Np++;
      Np--;
      break;

    case 3: // Incrementing Ki
      Ni++;
      break;

    case 6: // Decrementing Ki
      if (Ni == 0)
        Ni++;
      Ni--;
      break;

    case 0: // Reset
      Np = 0;
      Ni = 0;
      break;
    }

  default:
    break;
  }
}

// Timer 1 update
void CallbackMotor(void) {
  timerCount++;
  totalPulses = pulsesA + pulsesB;
  pulsesA = 0;
  pulsesB = 0;
}

// Rotary encoder A rising edge
void CallbackEncA(void) { pulsesA++; }

// Rotary encoder B rising edge
void CallbackEncB(void) { pulsesB++; }
