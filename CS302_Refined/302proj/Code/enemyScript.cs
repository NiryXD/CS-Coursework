using UnityEngine;

public class Enemy : MonoBehaviour
{
    public float moveSpeed = 5f;
    public float deadZone1 = -45f;
    public float deadZone2 = 45f;
    public ScoreManager manager;
    private int lastSpeedIncreaseScore = 0;

    void Start()
    {
        // assign ScoreManager if not set in inspector
        if (manager == null)
        {
            manager = FindFirstObjectByType<ScoreManager>();
            if (manager == null)
            {
                Debug.LogError("ScoreManager not found in the scene!");
            }
        }
    }

    void Update()
    {
        // move enemy left each frame
        transform.position = transform.position + (Vector3.left * moveSpeed) * Time.deltaTime;

        // delete enemy if it moves off screen
        if (transform.position.x < deadZone1 || transform.position.x > deadZone2)
        {
            Destroy(gameObject);
        }

        // increase enemy speed based on score
        if (manager != null)
        {
            int currentScore = manager.score;
            int currentScoreThreshold = (currentScore / 5) * 5;

            if (currentScoreThreshold > 0 && currentScoreThreshold > lastSpeedIncreaseScore)
            {
                moveSpeed += 2.5f * ((currentScoreThreshold - lastSpeedIncreaseScore) / 5);
                lastSpeedIncreaseScore = currentScoreThreshold;

                Debug.Log("Enemy speed increased to: " + moveSpeed + " at score: " + currentScore);
            }
        }
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        // deal damage to player if collided
        if (other.CompareTag("Player"))
        {
            PlayerHurtbox playerHurtbox = other.GetComponent<PlayerHurtbox>();

            if (playerHurtbox != null)
            {
                playerHurtbox.TakeDamage();
            }

            // destroy enemy on impact
            Destroy(gameObject);
        }
    }
}
