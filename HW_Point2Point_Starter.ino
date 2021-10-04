#include <Romi32U4.h>

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
struct robotPose getPose() {
  struct robotPose measuredP;
  measuredP.X = 0.8516*millis()/25.0;
  measuredP.Y = measuredP.X/1.732;
  measuredP.Q = 30.0;
  return measuredP;
}
void setup() {
  lcd.clear();
  Serial.begin(115200); // Serial COM at hight baud rate
  delay(2000);
}

void loop() {
  const float x_error = 0.1;
  const float y_error = 0.1;
  struct robotPose P = getPose(); // pretend this gives actual pose
  static int index = 0;
  static float x_way_pts[7] = {5, 10, 15, 20, 25}; // first way point is the starting position
  static float y_way_pts[7] = {30, 20, 15, 50, 100};
  static float last_x = 0;
  static float last_y = 0;

  static unsigned long millisLast = millis();
  float deltaT = 0.05;
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
  }

  // Doing some math to calculate the diff between the shortest path line
  float slope = (y_way_pts[index] - last_y) / (x_way_pts[index] - last_x);
  float intercept = last_y - slope * last_x;

  // Using a manipulated point-to-line distance formula
  float dist_err = (-1 * slope * P.X + P.Y - intercept) / sqrt(pow(slope, 2) + 1);
  int pwmDel = (int) (2.2 * dist_err);
  pwmDel = constrain(pwmDel, -100, 100);

  int pwmAvg = 50;
  if (
}
