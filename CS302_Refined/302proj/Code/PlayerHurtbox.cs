using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class PlayerHurtbox : MonoBehaviour
{
    [Header("Health")]
    public int health = 3;
    public Image[] hearts; // array for health UI

    [Header("Game Over UI")]
    public GameObject gameOverPanel;
    public Button restartButton;
    public Button quitButton;

    [Header("Audio")]
    public AudioSource backgroundMusic;
    public AudioSource gameOverMusic;
    public AudioSource damageSoundEffect;

    void Start()
    {
        // UI buttons
        restartButton.onClick.AddListener(RestartGame);
        quitButton.onClick.AddListener(QuitGame);
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        // lose health if hit by enemy
        if (other.CompareTag("Enemy"))
        {
            Destroy(other.gameObject);
            TakeDamage();
        }
    }

    public void TakeDamage()
    {
        if (health <= 0) return;

        health--;

        // disable heart icon to show damage
        if (health >= 0 && health < hearts.Length)
        {
            hearts[health].enabled = false;
        }

        damageSoundEffect.Play();

        // game over if out of health
        if (health <= 0)
        {
            Debug.Log("Player Died");
            Time.timeScale = 0f;
            gameOverPanel.SetActive(true);
            backgroundMusic.Stop();
            gameOverMusic.Play();
        }
    }

    public void RestartGame()
    {
        Time.timeScale = 1f;
        SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex);
    }

    public void QuitGame()
    {
        Time.timeScale = 1f;
        SceneManager.LoadScene("MainMenu");
    }
}
