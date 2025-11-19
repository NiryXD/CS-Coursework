using UnityEngine;
using System.Collections;

public class CharacterPunch : MonoBehaviour
{
    public Sprite idleSprite;
    public Sprite punchSprite;
    public float punchDuration = 0.2f;
    private SpriteRenderer spriteRenderer;
    public bool hasPunched = false;
    private bool facingRight = true;
    public Collider2D punchHitbox; 

    void Start()
    {
        spriteRenderer = GetComponent<SpriteRenderer>();
        spriteRenderer.sprite = idleSprite;

        if (punchHitbox != null)
            punchHitbox.enabled = false; // disable hitbox at start
    }

    void Update()
    {
        // flip character when changing direction
        if ((Input.GetKeyDown(KeyCode.A) || Input.GetKeyDown(KeyCode.LeftArrow)) && facingRight)
        {
            Flip();
        }
        else if ((Input.GetKeyDown(KeyCode.D) || Input.GetKeyDown(KeyCode.RightArrow)) && !facingRight)
        {
            Flip();
        }

        // start punch if not already punched
        if ((Input.GetKeyDown(KeyCode.A) || Input.GetKeyDown(KeyCode.D) ||
             Input.GetKeyDown(KeyCode.LeftArrow) || Input.GetKeyDown(KeyCode.RightArrow)) && !hasPunched)
        {
            StartCoroutine(Punch());
        }

        if ((Input.GetKeyDown(KeyCode.A) || Input.GetKeyDown(KeyCode.D) ||
             Input.GetKeyDown(KeyCode.LeftArrow) || Input.GetKeyDown(KeyCode.RightArrow)))
        {
            hasPunched = false;
        }
    }

    // flip character horizontally
    void Flip()
    {
        facingRight = !facingRight;
        Vector3 scale = transform.localScale;
        scale.x *= -1;
        transform.localScale = scale;
    }

    // punch animation and hitbox
    IEnumerator Punch()
    {
        hasPunched = true;
        spriteRenderer.sprite = punchSprite;

        if (punchHitbox != null)
            punchHitbox.enabled = true;

        yield return new WaitForSeconds(punchDuration);

        spriteRenderer.sprite = idleSprite;
        hasPunched = false;

        if (punchHitbox != null)
            punchHitbox.enabled = false;
    }
}
