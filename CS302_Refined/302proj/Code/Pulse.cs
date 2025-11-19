using UnityEngine;

public class PulseAndTilt : MonoBehaviour
{
    public float pulseSpeed = 1f;
    public float scaleAmount = 0.1f;

    public float tiltSpeed = 3f;
    public float tiltAmount = 5f;

    private Vector3 originalScale;
    private Quaternion originalRotation;

    void Start()
    {
        originalScale = transform.localScale;
        originalRotation = transform.rotation;
    }

    void Update()
    {
        // pulsing effect (growing and shrinking)
        float pulse = 1 + Mathf.Sin(Time.time * pulseSpeed) * scaleAmount;
        transform.localScale = originalScale * pulse;

        // tilt effect (rotating back and forth)
        float tilt = Mathf.Sin(Time.time * tiltSpeed) * tiltAmount;
        transform.rotation = Quaternion.Euler(0f, 0f, tilt);
    }
}
