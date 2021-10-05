#include <Romi32U4.h>
#include <math.h>

Romi32U4LCD lcd;
Romi32U4Buzzer buzzer;
Romi32U4ButtonA buttonA;
Romi32U4ButtonB buttonB;
Romi32U4ButtonC buttonC;
Romi32U4Motors motors;
Romi32U4Encoders encoders;

const char beepButtonA[] PROGMEM = "!c32";
const char beepButtonB[] PROGMEM = "!e32";
const char beepButtonC[] PROGMEM = "!g32";

// This function watches for button presses.  If a button is
// pressed, it beeps a corresponding beep and it returns 'A',
// 'B', or 'C' depending on what button was pressed.  If no
// button was pressed, it returns 0.  This function is meant to
// be called repeatedly in a loop.
char buttonMonitor()
{
  if (buttonA.getSingleDebouncedPress())
  {
    buzzer.playFromProgramSpace(beepButtonA);
    return 'A';
  }

  if (buttonB.getSingleDebouncedPress())
  {
    buzzer.playFromProgramSpace(beepButtonB);
    return 'B';
  }

  if (buttonC.getSingleDebouncedPress())
  {
    buzzer.playFromProgramSpace(beepButtonC);
    return 'C';
  }

  return 0;
}

// define data structure for robot pose
struct robotPose{
  double X;
  double Y;
  double Q;
};

//
// this is dummy code -- will be replaced by actual pose measurement
//
struct robotPose getPose(float deltaT, int L_robot) {
  static struct robotPose measuredP;
  float countLeft = encoders.getCountsAndResetLeft();
  float countRight = encoders.getCountsAndResetRight();
  float vLeft = PI * 6.9 * (countLeft/1440.0)/deltaT;
  float vRight = PI * 6.9 * (countRight/1440.0)/deltaT;
  float vAvg = (vLeft + vRight ) / 2;
  
  measuredP.X += deltaT * vAvg * cos(measuredP.Q);
  measuredP.Y += deltaT * vAvg * sin(measuredP.Q);
  measuredP.Q += deltaT * (vRight - vLeft) / L_robot;

  return measuredP;
}
void setup() {
  lcd.clear();
  Serial.begin(115200); // Serial COM at hight baud rate
  delay(2000);
}

void loop() {
  static float last_thetad = 0.0;
  const float x_error = 10;
  const float y_error = 10;
  const float slowdown_threshold = 15;
  float deltaT = 0.05;
  const int L_robot = 14;
  struct robotPose P = getPose(deltaT, L_robot); // pretend this gives actual pose
  static int index = 0;
  const int ARR_LEN = 7;
  static float x_way_pts[ARR_LEN] = {0, 63, 63, 0, 0}; // first way point is the starting position
  static float y_way_pts[ARR_LEN] = {0, 0, 63, 63, 0};
  static float last_x = 0;
  static float last_y = 0;

  static unsigned long millisLast = millis();
  while ((millis() - millisLast) < 1000 * deltaT) {}
  millisLast = millis();

  static byte motorsGo = 0;
  switch(buttonMonitor()) {
    case 'A':
      break;
    case 'B':
      motorsGo = 1-motorsGo;
      break;
    case 'C':
      break;
  }

  if (abs(P.X - x_way_pts[index]) < x_error and abs(P.Y - y_way_pts[index]) < y_error) {
    // TODO: turn towards new waypoint
    last_x = x_way_pts[index];
    last_y = y_way_pts[index];
    index++;

    if (index > ARR_LEN) {
      exit(0);
    }
  }

  int pwmAvg = 100;

  float theta_d = atan2(y_way_pts[index] - P.Y, x_way_pts[index] - P.X);
  float f = theta_d - P.Q;
  if (f > PI) {
    f = f - TWO_PI;
  } else if (f < -1 * PI) {
    f = f + TWO_PI;
  }

  int pwmDel = (int) (50.0 * f - 10.0 * (f - last_thetad)/deltaT;
  last_thetad = f;
  pwmDel = constrain(pwmDel, -100, 100);
  
  float dist_to_next = sqrt(pow(P.X - x_way_pts[index], 2) + pow(P.Y - y_way_pts[index], 2));
  if (dist_to_next < 10) {
    pwmAvg += (int) (-3.5 * (20 - dist_to_next));
  }

  if (motorsGo) {
    motors.setSpeeds(pwmAvg - pwmDel, pwmAvg + pwmDel);
  } else {
    motors.setSpeeds(0, 0);
  }
}
