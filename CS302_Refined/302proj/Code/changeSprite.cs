using UnityEngine;

public class EnemySpriteRandomizer : MonoBehaviour
{
    public Sprite[] possibleSprites; // array of sprites
    private SpriteRenderer sr;

    void Start()
    {
        sr = GetComponent<SpriteRenderer>();
        SetRandomSprite(); // random sprite at start
    }

    public void SetRandomSprite()
    {
        if (possibleSprites.Length > 0)
        {
            // choose random index and assign sprite
            int index = Random.Range(0, possibleSprites.Length);
            sr.sprite = possibleSprites[index];
        }
    }
}
