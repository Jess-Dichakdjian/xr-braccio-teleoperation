using System.Collections;
using System.IO.Ports;
using BioIK;
using UnityEngine;

/// <summary>
/// Bridges BioIK joint targets in Unity to an Arduino-controlled Braccio robot
/// using USB serial communication.
/// </summary>
public class BraccioBioIKBridge : MonoBehaviour
{
    [Header("BioIK")]
    public BioIK.BioIK bioIKSystem;

    [Header("BioIK Segment Names")]
    public string baseName = "Base";
    public string shoulderName = "Shoulder";
    public string elbowName = "Elbow";
    public string wristVerticalName = "WristVer";
    public string wristRotationName = "WristRot";

    [Header("Serial Communication")]
    public string portName = "COM3";
    public int baudRate = 115200;

    [Header("Command Rate")]
    [Range(0.05f, 0.5f)]
    public float sendInterval = 0.1f;

    private SerialPort serialPort;
    private bool isSending;

    private BioSegment baseSegment;
    private BioSegment shoulderSegment;
    private BioSegment elbowSegment;
    private BioSegment wristVerticalSegment;
    private BioSegment wristRotationSegment;

    private void Start()
    {
        OpenSerialConnection();
        FindBioIKSegments();

        isSending = true;
        StartCoroutine(SendJointCommands());
    }

    private void OpenSerialConnection()
    {
        serialPort = new SerialPort(portName, baudRate);

        try
        {
            serialPort.Open();
            Debug.Log($"Serial connection opened on {portName}.");
        }
        catch
        {
            Debug.LogWarning(
                $"Arduino not detected on {portName}. Running without hardware output."
            );
        }
    }

    private void FindBioIKSegments()
    {
        if (bioIKSystem == null)
        {
            Debug.LogError("BioIK system has not been assigned.");
            return;
        }

        baseSegment = bioIKSystem.FindSegment(baseName);
        shoulderSegment = bioIKSystem.FindSegment(shoulderName);
        elbowSegment = bioIKSystem.FindSegment(elbowName);
        wristVerticalSegment = bioIKSystem.FindSegment(wristVerticalName);
        wristRotationSegment = bioIKSystem.FindSegment(wristRotationName);
    }

    private IEnumerator SendJointCommands()
    {
        while (isSending)
        {
            int m1 = ToServoAngle(GetJointTarget(baseSegment, Axis.Y));
            int m2 = ToServoAngle(GetJointTarget(shoulderSegment, Axis.Z));
            int m3 = ToServoAngle(GetJointTarget(elbowSegment, Axis.Z));
            int m4 = ToServoAngle(GetJointTarget(wristVerticalSegment, Axis.Z));
            int m5 = ToServoAngle(GetJointTarget(wristRotationSegment, Axis.Y));

            // Fixed gripper position used in the final prototype.
            int m6 = 73;

            string command = $"{m1} {m2} {m3} {m4} {m5} {m6}";

            if (serialPort != null && serialPort.IsOpen)
            {
                serialPort.WriteLine(command);
            }

            Debug.Log($"Braccio command: {command}");

            yield return new WaitForSeconds(sendInterval);
        }
    }

    private enum Axis
    {
        X,
        Y,
        Z
    }

    private static double GetJointTarget(BioSegment segment, Axis axis)
    {
        if (segment?.Joint == null)
        {
            return 0.0;
        }

        switch (axis)
        {
            case Axis.X:
                return segment.Joint.X.GetTargetValue();

            case Axis.Y:
                return segment.Joint.Y.GetTargetValue();

            case Axis.Z:
                return segment.Joint.Z.GetTargetValue();

            default:
                return 0.0;
        }
    }

    private static int ToServoAngle(double bioIKDegrees)
    {
        return Mathf.RoundToInt(
            Mathf.Clamp((float)bioIKDegrees + 90f, 0f, 180f)
        );
    }

    public void SendUnityMode()
    {
        SendModeCommand("u");
    }

    public void SendHomeMode()
    {
        SendModeCommand("h");
    }

    private void SendModeCommand(string command)
    {
        if (serialPort != null && serialPort.IsOpen)
        {
            serialPort.WriteLine(command);
        }
    }

    private void OnApplicationQuit()
    {
        isSending = false;

        if (serialPort != null && serialPort.IsOpen)
        {
            serialPort.Close();
        }
    }
}
