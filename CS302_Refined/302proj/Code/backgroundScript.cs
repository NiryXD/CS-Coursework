using UnityEngine;

public class BackgroundCycler : MonoBehaviour
{
    public Sprite[] backgrounds; // background sprites array
    private SpriteRenderer sr;
    private int currentIndex = 0;
    private int lastCheckedScore = 0;

    void Start()
    {
        sr = GetComponent<SpriteRenderer>();
        if (backgrounds.Length > 0)
            sr.sprite = backgrounds[currentIndex]; // initial background
    }

    void Update()
    {
        int currentScore = ScoreManager.instance.score;

        // change background every 6 points
        if (currentScore != lastCheckedScore && currentScore % 6 == 0 && currentScore != 0)
        {
            lastCheckedScore = currentScore;
            CycleBackground();
        }
    }

    void CycleBackground()
    {
        // move to next background in array, looping back to start
        currentIndex = (currentIndex + 1) % backgrounds.Length;
        sr.sprite = backgrounds[currentIndex];
    }
}
