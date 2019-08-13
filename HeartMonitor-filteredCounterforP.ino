/* ADCpdbDMA
  PDB triggers the ADC which requests the DMA to move the data to a buffer
  Cheng Chun Kee 1673102
  Fabian H Sutandyo 1673089

*/

////////////////////////////////////////////////////////////////////// graph part
#define CS_PIN  8       // pin to detect touch
#include "SPI.h"
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include "ILI9341_t3.h"
// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

#include <font_Arial.h> // from ILI9341_t3
#include <XPT2046_Touchscreen.h>

XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN  2

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
//////////////////////////////////////////////////////////////////////

#define PDB_CH0C1_TOS 0x0100
#define PDB_CH0C1_EN 0x01

uint8_t ledOn = 0;

/////////////////////////////////////////////////////////// 3. Change sample into 30*250
/////////////////////////////////////////////////////////// because we need to save 30 sec of cardio trace data
uint16_t samples[7500];
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////moving average
float mav[10];
float mav2[10];
float mav3[10];
int counter;
int divider;
////////////////////////////////////////////////////////// 4. Generate fields for coordinate "counter"
uint16_t xAxis;
uint16_t yAxis;
////////////////////////////////////////////////////////// 5. Generate Field for touch
boolean wastouched;
////////////////////////////////////////////////////////////// set stabilization limit
uint16_t limit;
/////////////////////////////////////////////////////// loop on/off
boolean loopStart;
////////////////////////////////////filter constant
float v[5];
/////////////////// mmavOld
float mmavOld;
float inmmavOld;
////////////////// counterOld
float counterOld;
////////////////// counetrOld2
float counterOld2;
////////////////// BPM
float BPM;
//////////////////counta for stabilizing
int counta;
////////////////// find peak
float maxval;
///////////////// SPKI
float spki;
//////////////// npki
float npki;
///////////////threshold1
float thres1;
///////////////threshold2
float thres2;
//////////////peak
float peak1;
///////////////qrs start point
float qrsstart;
///////////////qrs end point
float qrsend;
///////////////qrs interval'
float qrstime;
////////////// filtered value
float filtered[7500];
float filtered2[7500];
////////////// rr average
float rraverage[8];
/////////////// divider2
int divider2;
float bpsignal;
int counterfilter;

float inmmav3Old;
float mmav3Old;
float counterOld3;
float maxS;

int sindex;
int counterfilter2;
float maxS2;
int divider3;
float mav4[10];

int counterS;
float pristart;
float priend;
int counterOld5;
int qindex;
float pritime;
int divider5;
float maxS5;
float mav5[10];
int counterp;
int counterfilter5;
int pindex;


void setup() {

  ////////////////////////////////////////////////////////////////////////////////////////// Graph part
  Wire.begin();
  Serial.begin(9600);


  adcInit();
  pdbInit();
  dmaInit();

  tft.begin();         // Initialize the display
  // tft.setContrast(58); // Set the contrast
  tft.setRotation(0);  //  0 or 2) width = width, 1 or 3) width = height, swapped etc.

  tft.fillScreen(ILI9341_WHITE);
  wastouched = true;
  loopStart = false;
  maxval = 0.0;
  counta = 0.0;

  //////////////////////////////////////////////////////////////////////////////////////////

}

void loop() {
  
  if (loopStart) {
    ///////////////////// wait to stabilize
     if (counta < 2000) {
      ////////////Write Stabilization GUI Here
      counta++;
     } else {
      /////////////////////////////////// find the max value of the filter
     for (int i = yAxis; i < yAxis + 200; i++) {
         for (int i = 0; i <= 4; i++) {
            v[i] = 0.0;
          }
          
        float bpsignal = bandpass(samples[i]);
    
         if (divider2 < 10) {
              divider2++;
            }
          
        mav3[counterfilter % 10] = bpsignal;
        float summav3 = sumArray(mav3, 10);
        float mmav3 = summav3 / divider2;
        float dmmav3 = Derivative(mmav3);
        float smmav3 = (dmmav3 * dmmav3); 
        float inmmav3 = MovingWindowIntegral(smmav3);
        
       if(mmav3 > maxval && mmav3 < 600 && mmav3 >= mmav3Old) {
               maxval = mmav3;      
         }
       if(inmmav3 > maxS && inmmav3 >= inmmav3Old) {
              maxS = inmmav3;
       }

       mmav3Old = mmav3;
       inmmav3Old = inmmav3;
       counterfilter++;
     }
     divider2 = 0;
     mmav3Old = 0.0;
     inmmav3Old = 0.0;
     
      ////////////////////////////////// Apply Band Pass Filter
      ///////////////// zignal = filtered version of samples[i]
      //////////////// zignal2 = filtered version of samples[ i + 1]
  
      for (int i = 0; i <= 4; i++) {
        v[i] = 0.0;
      }
      float zignal = bandpass(samples[yAxis]);
  
      for (int i = 0; i <= 4; i++) {
        v[i] = 0.0;
      }
      float zignal2 = bandpass(samples[yAxis + 1]);
  
      /////////////////////////////////////////////////////////////Moving average part
      ////////////////////////divider
      if (divider < 10) {
        divider++;
      }
  
      mav[counter % 10] = zignal;
      mav2[counter % 10] = zignal2;
  
      float summav = sumArray(mav, 10);
      float summav2 = sumArray(mav2, 10);
      ///////////////////////////////////////////////////////
      
      ////////////////////////////////////////////// graph this
      float mmav = summav / divider;
      float mmav2 = summav2 / divider;
      //////////////////////////////////////////////
  
      ////////////////////////////////////////////// derivation
      float dmmav = Derivative(mmav);
      float dmmav2 = Derivative(mmav2);
      ////////////////////////////////////////////////
      
      ///////////////////////////////////////////// squaring
      float smmav = (dmmav * dmmav);
      float smmav2 = (dmmav2 * dmmav2);
      /////////////////////////////////////////////
  
      /////////////////////////////////////////////// moving window integral
      float inmmav = MovingWindowIntegral(smmav);
      float inmmav2 = MovingWindowIntegral(smmav2);
      ////////////////////////////////////////////////
      filtered[yAxis%7500] = mmav;
      
      //////////////////////////////Draw a point for each pixel
      ///////////////////////////// This is the Cardio Reading
      tft.setRotation(0);
      tft.drawLine((mmav / 3) + 58 , xAxis  , (mmav2 / 3) + 58, xAxis + 1 , ILI9341_BLACK);
      //tft.drawLine((inmmav / 3) + 158 , xAxis  , (inmmav2 / 3) + 158, xAxis + 1 , ILI9341_BLACK);
  
      ////////////////////////////////////////////////////////////////////////////////////// R detection
      //////////////////////////////////////////////////////////////////////////////////////if statement is to detect the peak location using counter
      ////////////////////////////////////////////////////////////////////////////////////// it compares the current point with future point and past point
      ////////////////////////////////////////////////////////////////////////////////////// future point = mmav2; past point = mmavOld
      ////////////////////////////////////////////////////////////////////////////////////// if mmav is higher than those points then it's the PEAK
      if ( abs(mmav - maxval) <= 50.0 && (mmav >= thres1) && (mmav > mmavOld) && (counter - counterOld3 >= 150)&&(mmav > 300)) {
        tft.setRotation(0);
        tft.drawRect(120, xAxis+2, 25, 4, ILI9341_BLACK );
        tft.fillRect(120, xAxis+2, 25, 4, ILI9341_BLACK );
        counterOld3 = counter;
        
        BPM = 1 / ((counter - counterOld) / 250) * 60;
        
        ////////////////////////////////////////////////update spki
        spki = 0.125*filtered[yAxis%7500] + 0.875*spki;

        /////////////dtect Q point after detecting the peak
        for(int i = 5; i < 15; i++) {
          if ((filtered[yAxis-i] < filtered[yAxis-1-i]) && (filtered[yAxis-i] < filtered[yAxis-2-i])  && (filtered[yAxis-i] < filtered[yAxis+2-i]) && (filtered[yAxis-i] < filtered[yAxis+2-i]) && (counter - counterOld2 > 100) ) {
           tft.drawRect(120, xAxis - i- 1, 25, 2, ILI9341_GREEN );
           tft.fillRect(120, xAxis - i-1, 25, 2, ILI9341_GREEN );
           qindex = yAxis - i ;
           qrsstart = xAxis - i;
           
           counterOld2 = counter;
          }
        }
          
        /////////////detect P point
//           for(int i = 17; i < 15; i++) {
//          if ((filtered[qindex - i] > filtered[qindex-1-i]) && (filtered[qindex-i] > filtered[qindex-2-i])  && (filtered[qindex-i] > filtered[qindex+2-i]) && (filtered[qindex-i] > filtered[ qindex+2-i]) && (counter - counterOld5 > 100) ) {
//           tft.drawRect(120, xAxis - qindex - i - 1, 25, 2, ILI9341_YELLOW );
//           tft.fillRect(120, xAxis - qindex- i -1, 25, 2, ILI9341_YELLOW );
//           
//           pristart = xAxis - i - qindex;
//           counterOld3 = counter;
//          }
//        }

        if (qindex != 0) {
        for (int i = yAxis - 20 ; i < yAxis - 40 ; i--) {
             for (int i = 0; i <= 4; i++) {
                v[i] = 0.0;
              }
              
            float bpsignal5 = bandpass(samples[i]);
        
             if (divider5 < 10) {
                  divider5++;
                }
              
            mav5[counterfilter5 % 10] = bpsignal5;
            float summav5 = sumArray(mav5, 10);
            float mmav5 = summav5 / divider5;
          
            if(mmav5 > maxS5) {
                   maxS5 = mmav5;
                   pindex = counterp;      
             }
             counterp++;
           counterfilter5++;
         }
        tft.drawRect(120, xAxis - 20 - counterp - 2, 20, 4, ILI9341_YELLOW );
        tft.fillRect(120, xAxis - 20 - counterp - 2, 20, 4, ILI9341_YELLOW );
        pristart = xAxis - 20 - counterp;
        divider5 = 0;
        }

        
        ///////////detect S point
        
       for (int i = yAxis ; i < yAxis + 15 ; i++) {
             for (int i = 0; i <= 4; i++) {
                v[i] = 0.0;
              }
              
            float bpsignal4 = bandpass(samples[i]);
        
             if (divider3 < 10) {
                  divider3++;
                }
              
            mav4[counterfilter2 % 10] = bpsignal;
            float summav4 = sumArray(mav4, 10);
            float mmav4 = summav4 / divider2;
          
            if(mmav4 < maxS2) {
                   maxS2 = mmav4;
                   sindex = counterS;      
             }
             counterS++;
           counterfilter2++;
         }
        tft.drawRect(120, xAxis + counterS - 2, 20, 4, ILI9341_BLUE );
        tft.fillRect(120, xAxis + counterS - 2, 20, 4, ILI9341_BLUE );
        qrsend = xAxis + counterS;
        priend = xAxis + counterS;
        counterOld = counter;
        divider3 = 0;
        
         
      } else if (filtered[yAxis] <= thres1) {
        ////update npki
        npki = 0.125*filtered[yAxis%7500] + 0.875*npki;
      }
  
      
      inmmavOld = inmmav;
      mmavOld = mmav;
      ////////////////////////////////////////////////////////////////////////
  
  //////////////////calculate qrs time
   qrstime = (qrsend - qrsstart) / 250;
   ////////////////calculate pri time
   pritime = (priend - pristart)/250;
  
      ///////////////////////////// Printing the BPM
      if (counterOld != 0) {
        ////////////////////////////////////////////////////BPM rectangle(user interface)
        tft.setRotation(1);
        tft.drawRect(10, 160, 80, 40, ILI9341_WHITE);
        tft.fillRect(10, 160, 80, 40, ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK); // Set pixel color; 1 on the monochrome screen
        tft.setTextSize(2);
        tft.setCursor(20, 170);
        tft.print(BPM);
  
        /////////////////////////////////////////Bradycardia or Tachycardia
        if (BPM < 60) {
          tft.setTextColor(ILI9341_BLACK);
          tft.setTextSize(1);
          tft.setCursor(20, 200);
          tft.print("Bradycardia");
        }
        else if (BPM > 100) {
          tft.setTextColor(ILI9341_BLACK);
          tft.setTextSize(1);
          tft.setCursor(20, 200);
          tft.print("Tachycardia");
        }
        else {
          tft.setTextColor(ILI9341_BLACK);
          tft.setTextSize(1);
          tft.setCursor(40, 200);
          tft.print("Normal");
        }
  
       
      }
     }
  
  
      
  
    
    ////////////////////////////////increase counter;
      counter++;
  
      ////////////////////////////////shift the grapjh
      xAxis++;
      yAxis++; 
      maxval = 0.0;
      maxS = 0.0;
      maxS2 = 0.0;
      sindex = 0;
      counterS = 0;
      qindex = 0;
      counterp = 0;
      maxS5 = 0.0;
      
      //////////////////////////update the threshold
      thres1 = npki + 0.25*(spki - npki);
      thres2 = 0.5*thres1;

      ////////////////////////////////////////////////////////////////////////////////////////////////////Screen Order
      //Reset x coordinate and y coordinate after reach maximum value
      if (xAxis == 319) {
        xAxis = 0;
  
        tft.fillScreen(ILI9341_WHITE);
        tft.setRotation(0);
        for (int i = 0; i < 320; i = i + 15) {
          tft.drawLine(0, i, 240, i, ILI9341_RED);
        }
  
        for (int i = 0; i < 240; i = i + 15) {
          tft.drawLine(i, 0, i, 320, ILI9341_RED);
        }
  
        ////////////////////////////////////////////////////Big Black Rectangle(user interface)and white line
        tft.drawRect(0, 0, 120, 320, ILI9341_BLACK);
        tft.fillRect(0, 0, 120, 320, ILI9341_BLACK);
  
        tft.drawLine(0, 106, 120, 106, ILI9341_WHITE);
        tft.drawLine(0, 212, 120, 212, ILI9341_WHITE);
        /////////////////////////////////////////////////////
  
        //////////////////////////////////////////////////////Bottom left rectangle (Black and White)
        //////////////////////////////////////////////////////And Text: Heart Beat (BPM)
        tft.drawRect(5, 5, 110, 96, ILI9341_WHITE);
        tft.fillRect(5, 5, 110, 96, ILI9341_WHITE);//White one
  
        tft.setRotation(1);
  
        tft.setTextColor(ILI9341_BLACK);
        tft.setTextSize(1);
        tft.setCursor(20, 140);
        tft.print("Heart Beat");
        tft.setCursor(35, 150);
        tft.print("(BPM)");// Text
        ///////////////////////////////////////////////////////
  
        /////////////////////////////////////////////////////// Bottom middle rectangle (Black and White)
  
        tft.drawRect(111, 125, 96, 110, ILI9341_WHITE);
        tft.fillRect(111, 125, 96, 110, ILI9341_WHITE);//White one
  
        tft.setTextColor(ILI9341_BLACK);
        tft.setTextSize(1);
        tft.setCursor(114, 145);
        tft.print("PRI: ");
        tft.setCursor(150,145);
        tft.print(pritime);
        tft.setCursor(185, 145);
        tft.print("Sec");
        tft.setCursor(114, 200);
        tft.print("QRS: ");
        tft.setCursor(150,200);
        tft.print(qrstime);
        tft.setCursor(185, 200);
        tft.print("Sec");
  
      }
  
      if (yAxis == 7499) {
        yAxis = 0;
      }
      //Serial.println(" ");
  
      //}
      Serial.println(" ");
      loopStart = false;
  }
}

static const uint8_t channel2sc1a[] = {
  5, 14, 8, 9, 13, 12, 6, 7, 15, 4,
  0, 19, 3, 21, 26, 22
};

/*
	ADC_CFG1_ADIV(2)         Divide ratio = 4 (F_BUS = 48 MHz => ADCK = 12 MHz)
	ADC_CFG1_MODE(2)         Single ended 10 bit mode
	ADC_CFG1_ADLSMP          Long sample time
*/

//////////////////////////////////////////////////////////// 1. Change bit resolution to 12-bits
///////////////////////////////////////////////////////////  by changing ADC_CFG1_Mode from 2 to 1
/////////////////////////////////////////////////////////// because (2) = 10 bits and (1) = 12 bits
#define ADC_CONFIG1 (ADC_CFG1_ADIV(1) | ADC_CFG1_MODE(1) | ADC_CFG1_ADLSMP)
///////////////////////////////////////////////////////////

/*
	ADC_CFG2_MUXSEL          Select channels ADxxb
	ADC_CFG2_ADLSTS(3)       Shortest long sample time
*/
#define ADC_CONFIG2 (ADC_CFG2_MUXSEL | ADC_CFG2_ADLSTS(3))

void adcInit() {
  ADC0_CFG1 = ADC_CONFIG1;
  ADC0_CFG2 = ADC_CONFIG2;
  // Voltage ref vcc, hardware trigger, DMA
  ADC0_SC2 = ADC_SC2_REFSEL(0) | ADC_SC2_ADTRG | ADC_SC2_DMAEN;

  // Enable averaging, 4 samples
  ADC0_SC3 = ADC_SC3_AVGE | ADC_SC3_AVGS(0);

  adcCalibrate();
  Serial.println("calibrated");

  // Enable ADC interrupt, configure pin
  ADC0_SC1A = ADC_SC1_AIEN | 5; /////Using pin no 14 (channel12sc1a[1]) for ADC interrupt Pin
  NVIC_ENABLE_IRQ(IRQ_ADC0);
}

void adcCalibrate() {
  uint16_t sum;

  // Begin calibration
  ADC0_SC3 = ADC_SC3_CAL;
  // Wait for calibration
  while (ADC0_SC3 & ADC_SC3_CAL);

  // Plus side gain
  sum = ADC0_CLPS + ADC0_CLP4 + ADC0_CLP3 + ADC0_CLP2 + ADC0_CLP1 + ADC0_CLP0;
  sum = (sum / 2) | 0x8000;
  ADC0_PG = sum;

  // Minus side gain (not used in single-ended mode)
  sum = ADC0_CLMS + ADC0_CLM4 + ADC0_CLM3 + ADC0_CLM2 + ADC0_CLM1 + ADC0_CLM0;
  sum = (sum / 2) | 0x8000;
  ADC0_MG = sum;
}

/*
	PDB_SC_TRGSEL(15)        Select software trigger
	PDB_SC_PDBEN             PDB enable
	PDB_SC_PDBIE             Interrupt enable
	PDB_SC_CONT              Continuous mode
	PDB_SC_PRESCALER(7)      Prescaler = 128
	PDB_SC_MULT(1)           Prescaler multiplication factor = 10
*/
#define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE \
                    | PDB_SC_CONT | PDB_SC_PRESCALER(7) | PDB_SC_MULT(1))

// 48 MHz / 128 / 10 / 1 Hz = 37500
#define PDB_PERIOD (F_BUS / 128 / 10 / 1)


void pdbInit() {
  pinMode(13, OUTPUT);

  // Enable PDB clock
  SIM_SCGC6 |= SIM_SCGC6_PDB;

  // Timer period
  /////////////////////////////////////////////////////////// 2. Change sample Rate to 250 Hz
  PDB0_MOD = PDB_PERIOD / 250;
  /////////////////////////////////////////////////////////

  // Interrupt delay
  PDB0_IDLY = 0;
  // Enable pre-trigger
  PDB0_CH0C1 = PDB_CH0C1_TOS | PDB_CH0C1_EN;
  // PDB0_CH0DLY0 = 0;
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK;
  // Software trigger (reset and restart counter)
  PDB0_SC |= PDB_SC_SWTRIG;
  // Enable interrupt request
  NVIC_ENABLE_IRQ(IRQ_PDB);
}

void dmaInit() {
  // Enable DMA, DMAMUX clocks
  SIM_SCGC7 |= SIM_SCGC7_DMA;
  SIM_SCGC6 |= SIM_SCGC6_DMAMUX;

  // Use default configuration
  DMA_CR = 0;

  // Source address
  DMA_TCD1_SADDR = &ADC0_RA;
  // Don't change source address
  DMA_TCD1_SOFF = 0;
  DMA_TCD1_SLAST = 0;
  // Destination address
  DMA_TCD1_DADDR = samples;
  // Destination offset (2 byte)
  DMA_TCD1_DOFF = 2;
  // Restore destination address after major loop
  DMA_TCD1_DLASTSGA = -sizeof(samples);
  // Source and destination size 16 bit
  DMA_TCD1_ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);
  // Number of bytes to transfer (in each service request)
  DMA_TCD1_NBYTES_MLNO = 2;
  // Set loop counts
  DMA_TCD1_CITER_ELINKNO = sizeof(samples) / 2;
  DMA_TCD1_BITER_ELINKNO = sizeof(samples) / 2;
  // Enable interrupt (end-of-major loop)
  DMA_TCD1_CSR = DMA_TCD_CSR_INTMAJOR;

  // Set ADC as source (CH 1), enable DMA MUX
  DMAMUX0_CHCFG1 = DMAMUX_DISABLE;
  DMAMUX0_CHCFG1 = DMAMUX_SOURCE_ADC0 | DMAMUX_ENABLE;

  // Enable request input signal for channel 1
  DMA_SERQ = 1;

  // Enable interrupt request
  NVIC_ENABLE_IRQ(IRQ_DMA_CH1);
}


void adc0_isr() {
  loopStart = true;
}

void pdb_isr() {
  //Serial.print("pdb isr: ");
  //Serial.println(millis());
  //digitalWrite(13, (ledOn = !ledOn));
  // Clear interrupt flag
  PDB0_SC &= ~PDB_SC_PDBIF;
}

void dma_ch1_isr() {
  //Serial.print("dma isr: ");
  //Serial.println(millis());
  // Clear interrupt request for channel 1
  DMA_CINT = 1;
}

////////////////////////////////////////////////////////////////////////filter functions
float bandpass(float x) //class II
{
  v[0] = v[1];
  v[1] = v[2];
  v[2] = v[3];
  v[3] = v[4];
  v[4] = (1.454429670328223900e-1 * x)
         + (-0.25610643855067372066 * v[0])
         + (1.19029069365812611991 * v[1])
         + (-2.60211112952912948515 * v[2])
         + (2.66783480391750549288 * v[3]);
  return
    (v[0] + v[4])
    - 2 * v[2];
}
int Derivative(int data)
{
  int y, i;
  static int x_derv[4];
  /*y = 1/8 (2x( nT) + x( nT - T) - x( nT - 3T) - 2x( nT -
    4T))*/
  y = (data << 1) + x_derv[3] - x_derv[1] - ( x_derv[0] <<
      1);
  y >>= 3;
  for (i = 0; i < 3; i++)
    x_derv[i] = x_derv[i + 1];
  x_derv[3] = data;
  return (y);
}

int MovingWindowIntegral(int data)
{
  static int x[32], ptr = 0;
  static long sum = 0;
  long ly;
  int y;
  if (++ptr == 32)
    ptr = 0;
  sum -= x[ptr];
  sum += data;
  x[ptr] = data;
  ly = sum >> 5;
  if (ly > 32400) /*check for register overflow*/
    y = 32400;
  else
    y = (int) ly;
  return (y);
}
//////////////////////////////////////////////////////////////////

//Helper method for calculatin the sum of an array
float sumArray(float array1[], int n) {
  float sum = 0;
  for (int i = 0; i < n; i++) {
    sum += array1[i];
  }
  return sum;
}



