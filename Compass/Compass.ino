#include "Wire.h"
#include "U8glib.h"


//-------HMC5883L-------//
//#include "HMC5883L.h"
//HMC5883L compass;


//-------QMC5883-------//
#include <DFRobot_QMC5883.h>
DFRobot_QMC5883 compass;


float xv, yv, zv;
float heading;
float headingDegrees;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);    // Fast I2C / TWI 

//-------HMC5883L-------//
//void setupHMC5883L()
//{  
//  compass.SetScale(0.88);
//  compass.SetMeasurementMode(Measurement_Continuous);
//}

//calibrated_values[3] is the global array where the calibrated data will be placed
//calibrated_values[3]: [0]=Xc, [1]=Yc, [2]=Zc
float calibrated_values[3];   
//transformation(float uncalibrated_values[3]) is the function of the magnetometer data correction 
//uncalibrated_values[3] is the array of the non calibrated magnetometer data
//uncalibrated_values[3]: [0]=Xnc, [1]=Ync, [2]=Znc
void transformation(float uncalibrated_values[3])    
{
  //calibration_matrix[3][3] is the transformation matrix
  //replace M11, M12,..,M33 with your transformation matrix data
  double calibration_matrix[3][3] = 
  {
    { 1.103,  -0.105,  -0.038},
    { 0.038,    1.09,  -0.003},
    {-0.005,       0,   1.138}  
  };
  //bias[3] is the bias
  //replace Bx, By, Bz with your bias data
  double bias[3] = 
  {
    1697.201,
    1001.918,
    2443.618
  };  
  //calculation
  for (int i=0; i<3; ++i) uncalibrated_values[i] = uncalibrated_values[i] - bias[i];
  float result[3] = {0, 0, 0};
  for (int i=0; i<3; ++i)
    for (int j=0; j<3; ++j)
      result[i] += calibration_matrix[i][j] * uncalibrated_values[j];
  for (int i=0; i<3; ++i) calibrated_values[i] = result[i];
}

//vector_length_stabilasation() - is the function of the magnetometer vector length stabilasation (stabilisation of the sphere radius)
float scaler;
boolean scaler_flag = false;
float normal_vector_length;
void vector_length_stabilasation(){
  //calculate the normal vector length
  if (scaler_flag == false)
  {
    getHeading();
    normal_vector_length = sqrt(calibrated_values[0]*calibrated_values[0] + calibrated_values[1]*calibrated_values[1] + calibrated_values[2]*calibrated_values[2]);
    scaler_flag = true;
  } 
  //calculate the current scaler
  scaler = normal_vector_length/sqrt(calibrated_values[0]*calibrated_values[0] + calibrated_values[1]*calibrated_values[1] + calibrated_values[2]*calibrated_values[2]);
  //apply the current scaler to the calibrated coordinates (global array calibrated_values)
  calibrated_values[0] = calibrated_values[0]*scaler;
  calibrated_values[1] = calibrated_values[1]*scaler;
  calibrated_values[2] = calibrated_values[2]*scaler;
}

void setup()
{   
    Serial.begin(115200);
    Wire.begin(); 
    
    //--------------HMC5883L------------//
    //compass = HMC5883L();             //
    //setupHMC5883L();                  //
     
    
    //--------------------------------QMC5883-----------------------------------//
    while (!compass.begin()){                                                   //
        Serial.println("Could not find a valid QMC5883 sensor, check wiring!"); //
        delay(500);                                                             //
    }                                                                           //
    compass.setRange(QMC5883_RANGE_2GA);                                        //
    compass.setMeasurementMode(QMC5883_CONTINOUS);                              //
    compass.setDataRate(QMC5883_DATARATE_50HZ);                                 //
    compass.setSamples(QMC5883_SAMPLES_8);                                      //
}

void loop()
{
    float values_from_magnetometer[3];

    getHeading();                             //
    values_from_magnetometer[0] = xv;
    values_from_magnetometer[1] = yv;
    values_from_magnetometer[2] = zv;
    
    transformation(values_from_magnetometer);
    
    vector_length_stabilasation();

    //float heading = atan2(calibrated_values[0], calibrated_values[1]);
    heading = atan2(calibrated_values[0], -calibrated_values[1]);
    float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
    heading += declinationAngle;

    if (heading < 0){
        heading += 2 * PI;
    }
    
    if (heading > 2 * PI){
        heading -= 2 * PI;
    }
    
    // Convert to degrees
    headingDegrees = heading * 180/PI; 
    
    // Output
    Serial.print(" Heading = ");
    Serial.print(heading);
    Serial.print(" Degress = ");
    Serial.print(headingDegrees);
    Serial.println();


    
    /*Serial.flush(); 
    Serial.print(calibrated_values[0]); 
    Serial.print(",");
    Serial.print(calibrated_values[1]);
    Serial.print(",");
    Serial.print(calibrated_values[2]);
    Serial.println();
    */

    u8g.firstPage();  
    do {
        draw();
    } while( u8g.nextPage() );
    
    delay(100); 
} 


 
void getHeading()
{ 
    //-------------------HMC5883L---------------//
    //MagnetometerRaw raw = compass.ReadRawAxis();

    //-------------------QMC5883----------------//
    Vector raw = compass.readRaw();
    
    xv = (float)raw.XAxis;
    yv = (float)raw.YAxis;
    zv = (float)raw.ZAxis;
}

void draw(void) {
  static int armLength = 20;
  static int cx = 64;
  static int cy = 20;
  int armX, armY;
  
  //convert degree to radian
  //float bearingRad = headingDegrees/57.2957795;
  armY = armLength*cos(heading);
  armX = armLength*sin(heading);

  u8g.setFont(u8g_font_unifont);
  
  u8g.setPrintPos(0, 60);
  u8g.print("bearing: ");
  u8g.setPrintPos(70, 60);
  u8g.print(headingDegrees);
  
  u8g.drawLine(cx, cy, cx-armX, cy-armY);
  u8g.drawCircle(cx, cy, armLength, U8G_DRAW_ALL);

}

