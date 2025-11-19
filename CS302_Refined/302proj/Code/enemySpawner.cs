using UnityEngine;

public class EnemySpawner : MonoBehaviour
{
    public GameObject[] enemyPrefabs;   // array of enemy types
    public float spawnInterval = 2f;
    private float timer = 0f;

    void Update()
    {
        timer += Time.deltaTime;

        // spawn new enemy when interval is reached
        if (timer >= spawnInterval)
        {
            SpawnEnemy();
            timer = 0f;
        }
    }

    void SpawnEnemy()
    {
        // randomly choose whether to spawn from left or right
        bool spawnFromRight = Random.value > 0.5f;

        float spawnX = spawnFromRight ? 15f : -15f;
        Vector3 spawnPosition = new Vector3(spawnX, -2f, -3f);

        // choose random enemy prefab
        int index = Random.Range(0, enemyPrefabs.Length);
        GameObject enemy = Instantiate(enemyPrefabs[index], spawnPosition, Quaternion.identity);

        // flip/reverse movement if spawning from left
        if (!spawnFromRight)
        {
            enemy.transform.localScale = new Vector3(-enemy.transform.localScale.x, enemy.transform.localScale.y, enemy.transform.localScale.z);
            enemy.GetComponent<Enemy>().moveSpeed *= -1;
        }
    }
}
