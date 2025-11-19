using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;
using UnityEngine.Audio;

public class StartMenuManager : MonoBehaviour
{
    public Button startButton;
    public Button settingsButton;
    public Button quitButton;

    public GameObject mainMenuPanel;
    public GameObject settingsPanel;
    public GameObject logoObject;

    public Slider volumeSlider;
    public Toggle fullscreenToggle;
    public Button backButton;

    public AudioMixer audioMixer;

    void Start()
    {
        ShowMainMenu(); // show the main menu at start

        // button listeners for functionality
        if (startButton != null)
            startButton.onClick.AddListener(StartGame);

        if (settingsButton != null)
            settingsButton.onClick.AddListener(OpenSettings);

        if (quitButton != null)
            quitButton.onClick.AddListener(QuitGame);

        if (backButton != null)
            backButton.onClick.AddListener(ShowMainMenu);

        // volume slider
        if (volumeSlider != null)
        {
            // load/apply saved volume setting
            float savedVolume = PlayerPrefs.GetFloat("MasterVolume", 1.0f);
            volumeSlider.value = savedVolume;
            SetVolume(savedVolume);
            volumeSlider.onValueChanged.AddListener(SetVolume);
        }

        // fullscreen toggle
        if (fullscreenToggle != null)
        {
            fullscreenToggle.isOn = Screen.fullScreen;
            fullscreenToggle.onValueChanged.AddListener(SetFullscreen);
        }
    }

    // start game by loading "GameScene"
    public void StartGame()
    {
        SceneManager.LoadScene("GameScene");
    }

    // open settings panel and hide main menu
    public void OpenSettings()
    {
        if (mainMenuPanel != null)
            mainMenuPanel.SetActive(false);

        if (settingsPanel != null)
            settingsPanel.SetActive(true);

        if (logoObject != null)
            logoObject.SetActive(false);
    }

    // show main menu and hide settings panel
    public void ShowMainMenu()
    {
        if (mainMenuPanel != null)
            mainMenuPanel.SetActive(true);

        if (settingsPanel != null)
            settingsPanel.SetActive(false);

        if (logoObject != null)
            logoObject.SetActive(true);
    }

    // set volume based on slider value
    public void SetVolume(float volume)
    {
        PlayerPrefs.SetFloat("MasterVolume", volume); // save volume setting

        // volume to decibels and set in audio mixer
        float dB = Mathf.Log10(Mathf.Clamp(volume, 0.0001f, 1f)) * 20f;
        audioMixer.SetFloat("MasterVolume", dB);

        Debug.Log("Volume set to: " + volume + " (" + dB + " dB)");
    }

    // set fullscreen mode based on toggle value
    public void SetFullscreen(bool isFullscreen)
    {
        Screen.fullScreen = isFullscreen;
    }

    // quit game
    public void QuitGame()
    {
        Debug.Log("Quit Game");
#if UNITY_EDITOR
        UnityEditor.EditorApplication.isPlaying = false; // stop play mode when in editor
#else
            Application.Quit(); // quit when built
#endif
    }
}
