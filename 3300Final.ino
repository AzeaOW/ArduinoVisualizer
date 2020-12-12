//LED library: https://www.arduinolibraries.info/libraries/md_max72-xx
//FFT Library: https://www.arduinolibraries.info/libraries/arduino-fft
//code cited from :https://github.com/AlexFWulff/MusicVisualizer/blob/master/MusicVisualizer.ino
//                 https://www.makerguides.com/max7219-led-dot-matrix-display-arduino-tutorial/
//Along with tutorials on Library links




#include <arduinoFFT.h> 
#include <MD_MAX72xx.h>

#define SAMPLES 64  //number of samples for fft stuff
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW   //init led type for mx lib
#define MAX_DEVICES  4   //4 8x8 LEDS
#define CLK_PIN   13  
#define DATA_PIN  11 
#define CS_PIN    10  
#define ledx 32 //leds width
#define ledy 8 //leds height

int DISPLAY_VALS[]={0, 128, 192, 224, 240, 248, 252, 254, 255}; //ideal default frequency values for standard visualizer
double vReal[SAMPLES];
double vImag[SAMPLES];
char data_avgs[ledx];
int yvalue;
int displaycolumn , displayvalue;
int peaks[ledx]; 

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);   // display 
arduinoFFT FFT = arduinoFFT(); //fft

void setup() {
    ADCSRA = 0b11100101;      // ADC conversion - free run mode 
    ADMUX = 0b00000000;       // A0 pin and use external voltage
    mx.begin();           // initialize display
    delay(50);

}

void loop() {
  // step 1: sample the audio
  for(int i=0; i<SAMPLES; i++)
    {
      while(!(ADCSRA & 0x10));        // wait for ADC to conversion
      ADCSRA = 0b11110101 ;               // clear ADIF so that it can move on to the next
      int value = ADC - 512 ;                 // subtract offset of 512
      vReal[i]= value/8;                      // Copy & compress
      vImag[i] = 0;   
      Serial.println(vImag[i]);
    }

    // step 2: use FFT algorithm
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD); //window it
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD); //compute it
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES); //convert it


    // step 3: match the data to the columns of the LED
     int step = (SAMPLES/2)/ledx; 
    int col=0;
    for(int i=0; i<(SAMPLES/2); i+=step)  
    {
      data_avgs[col] = 0;
      for (int k=0 ; k< step ; k++) {
          data_avgs[col] = data_avgs[col] + vReal[i+k];
      }
      data_avgs[col] = data_avgs[col]/step; 
      col++;
    }

    
    //step 4: Send data to LED Matrix
    
     for(int i=0; i<ledx; i++)
      {
      data_avgs[i] = constrain(data_avgs[i],0,80);         
      data_avgs[i] = map(data_avgs[i], 0, 80, 0, ledy);        // remap averaged values to Y of leds
      yvalue=data_avgs[i];

      peaks[i] = peaks[i]-1;    // decay by one light
      if (yvalue > peaks[i]) 
          peaks[i] = yvalue ;
      yvalue = peaks[i];    
      displayvalue=DISPLAY_VALS[yvalue];
      displaycolumn=31-i;
      mx.setColumn(displaycolumn, displayvalue);          
     }
}
