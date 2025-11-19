using UnityEngine;
using UnityEngine.SceneManagement;

public class PunchHitbox : MonoBehaviour
{
    public int pointsPerEnemy = 1;

    void OnTriggerEnter2D(Collider2D other)
    {
        // if collided with object with "Enemy" tag
        if (other.CompareTag("Enemy"))
        {
            Destroy(other.gameObject); // delete enemy 
            ScoreManager.instance.AddScore(pointsPerEnemy); // add to score
        }
    }
}
