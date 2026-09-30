#include <Servo.h>
#include <Braccio.h>

// Braccio servo objects
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo wristVerticalServo;
Servo wristRotationServo;
Servo gripperServo;

// Servo pins used by the Braccio shield
const int servoPins[] = {11, 10, 9, 6, 5, 3};

enum ControlMode
{
    MODE_HOME,
    MODE_MANUAL,
    MODE_UNITY
};

ControlMode currentMode = MODE_HOME;

void setup()
{
    Serial.begin(115200);

    // Enable Braccio shield
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);

    baseServo.attach(servoPins[0]);
    shoulderServo.attach(servoPins[1]);
    elbowServo.attach(servoPins[2]);
    wristVerticalServo.attach(servoPins[3]);
    wristRotationServo.attach(servoPins[4]);
    gripperServo.attach(servoPins[5]);

    printMenu();
    moveHome();
}

void loop()
{
    if (Serial.available() <= 0)
    {
        return;
    }

    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() == 0)
    {
        return;
    }

    // Control-mode commands
    if (command == "h")
    {
        currentMode = MODE_HOME;
        moveHome();
        printMenu();
    }
    else if (command == "m")
    {
        currentMode = MODE_MANUAL;
        Serial.println("--- MANUAL MODE ACTIVE ---");
        printMenu();
    }
    else if (command == "u")
    {
        currentMode = MODE_UNITY;
        Serial.println("--- UNITY MODE ACTIVE ---");
        printMenu();
    }
    else
    {
        if (currentMode == MODE_MANUAL)
        {
            executeManualCommand(command);
        }
        else if (currentMode == MODE_UNITY)
        {
            executeUnityCommand(command);
        }
    }
}


void moveHome()
{
    Serial.println("Moving to HOME position...");

    baseServo.write(90);
    shoulderServo.write(45);
    elbowServo.write(180);
    wristVerticalServo.write(180);
    wristRotationServo.write(90);
    gripperServo.write(10);

    delay(500);
}


void executeManualCommand(String command)
{
    int values[2];
    int count = 0;

    char buffer[32];
    command.toCharArray(buffer, sizeof(buffer));

    char *token = strtok(buffer, " ");

    while (token != nullptr && count < 2)
    {
        values[count++] = atoi(token);
        token = strtok(nullptr, " ");
    }

    if (count != 2)
    {
        return;
    }

    int motor = values[0];
    int angle = values[1];

    if (motor < 1 || motor > 6 || angle < 0 || angle > 180)
    {
        Serial.println(
            "Error: motor must be 1-6 and angle must be 0-180."
        );
        return;
    }

    switch (motor)
    {
        case 1:
            baseServo.write(angle);
            break;

        case 2:
            shoulderServo.write(angle);
            break;

        case 3:
            elbowServo.write(angle);
            break;

        case 4:
            wristVerticalServo.write(angle);
            break;

        case 5:
            wristRotationServo.write(angle);
            break;

        case 6:
            gripperServo.write(angle);
            break;
    }

    Serial.print("Motor ");
    Serial.print(motor);
    Serial.print(" moved to ");
    Serial.println(angle);
}


void executeUnityCommand(String command)
{
    int jointAngles[6];
    int count = 0;

    char buffer[80];
    command.toCharArray(buffer, sizeof(buffer));

    char *token = strtok(buffer, " ");

    while (token != nullptr && count < 6)
    {
        jointAngles[count++] = atoi(token);
        token = strtok(nullptr, " ");
    }

    if (count != 6)
    {
        return;
    }

    baseServo.write(jointAngles[0]);
    shoulderServo.write(jointAngles[1]);
    elbowServo.write(jointAngles[2]);
    wristVerticalServo.write(jointAngles[3]);
    wristRotationServo.write(jointAngles[4]);
    gripperServo.write(jointAngles[5]);
}


void printMenu()
{
    Serial.println("\n--- BRACCIO CONTROL MENU ---");
    Serial.println("h: HOME mode");
    Serial.println("m: MANUAL mode");
    Serial.println("u: UNITY mode");

    Serial.print("Current mode: ");

    if (currentMode == MODE_HOME)
    {
        Serial.println("HOME");
    }
    else if (currentMode == MODE_MANUAL)
    {
        Serial.println("MANUAL");
    }
    else if (currentMode == MODE_UNITY)
    {
        Serial.println("UNITY");
    }
}
