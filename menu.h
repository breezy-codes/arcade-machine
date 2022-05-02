
using namespace std;
using std::vector;

class Menu {
private:
    string background = "games_dashboard";
    // Vector to store the config data of each game
    vector<ConfigData> _games;
    // Contains info about newly created process and thread
    PROCESS_INFORMATION processInfo;
    // Unsigned int to store exit info
    DWORD exit_code;
    // Holds the game path of selected game
    LPCSTR _gamePath;
    // Holds the executable of selected game
    LPSTR _gameExe;
    // Holds the game directory of selected game
    LPCSTR _gameDir;
    // Used to find x centre of screen
    double _center_x = 960;
    // Used to fine y centre of screen
    double _center_y = 540;
    // Stores x of mouse position
    double _x;
    // Stores y of mouse position
    double _y;
    // Checks if game is running
    bool _in_game = false;
    // Checks if program has exited
    bool _program_exit;
    // Vector of buttons
    vector<Button*> btns;
    // A selector
    Selector select;
    // The action to take
    string action;
    // Vectore to store game images
    vector<string> game_images;
    // Menu grid
    Grid _grid;
    ButtonNode *button = nullptr;
    bool _overlayActive = false;
    /// Button Action 
    string _action;
    Selector _selector_games_menu;
    // Passes into Selector optional parameter.
    bool game_menu = true;

public:
    Menu(){}

    Menu(vector<ConfigData> configs)
    {
        this->_games = configs;
    }
    ~Menu(){}

    // Getters
    auto get_buttons() const -> const vector<Button*> { return this->btns; }
    bool get_overlay_state() { return _overlayActive; }

    // This function gets the game images from the config files and returns vector of game images.
    vector<string> get_game_sprites(vector<ConfigData> configs)
    {
        vector<string> game_images;

        for (int i = 0; i < configs.size(); i++)
        {
            // Get image dir and image name from games vector.
            string image = configs[i].folder() + "/" + configs[i].image();
            game_images.push_back(image);
        }

        return game_images;
    }

    void create_grid()
    {
        // Instantiate grid object
        Grid grid(8, 14);
        this->_grid = grid;
        // Update the background
        this->_grid.SetBackground(bitmap_named(this->background));
    }

    // This function creates the game buttons from the game images.
    void create_buttons()
    {
        // Call function to get game images.
        game_images = get_game_sprites(_games);

        for (int i = 0; i < game_images.size(); i++)
        {
            if (i == 0)
            {
                this->button = new ButtonNode(new GameScreenButton(Button::GAME, game_images[0]));
                this->button->config = _games[0];
            }
            else
            {
                string image = game_images[i];
                this->button->addBefore(new ButtonNode(new GameScreenButton(Button::GAME, image)));
                this->button->getPrev()->config = _games[i];
            }
        }
    }

    // Draw the game buttons to the window, using the carousel layout
    void update_carousel()
    {
        if (this->button)
        {
            this->_grid.UpdateCell(this->button->getPrev()->button, 2, 0, 1, false);
            this->_grid.UpdateCell(this->button->button, 2, 5, 1, false);
            this->_grid.UpdateCell(this->button->getNext()->button, 2, 10, 1, false);
        }
    }

    //Handle carousel input
    void carousel_handler()
    {
        /// Check for input in selector class.
        this->button = this->_selector_games_menu.check_key_input(this->button, game_menu);

        if (this->button)
        {
            if (key_typed(ESCAPE_KEY) && _overlayActive)
            {
                _overlayActive = false;
            }
            else if (key_typed(RETURN_KEY))
            {
                if (_overlayActive)
                {
                    // Get game path
                    _gamePath = (this->button->config.folder() + "/" + this->button->config.exe()).c_str();
                    // Get executable name
                    _gameExe = strdup(this->button->config.exe().c_str());
                    // Get game directory
                    _gameDir = this->button->config.folder().c_str();

                    // Set the center of the game
                    this->_x = _center_x;
                    this->_y = _center_y;

                    // Call method to open game executable
                    start_game(_gamePath, _gameExe, _gameDir);
                    return;
                }
                _overlayActive = true;
            }
        }
    }

    // Draw the background and call set game image.
    void draw_menu_page()
    {
        carousel_handler();
        update_carousel();
        this->_grid.DrawGrid();
        if (_overlayActive)
            draw_overlay(button->config);
    }
    void draw_overlay(ConfigData config)
    {
        int x_offset = (current_window_width() / 2) + (current_window_width() / 14);
        int y_start = current_window_height() / 6;
        int y_offset = current_window_height() / 40;

        fill_rectangle(rgba_color(0.0, 0.0, 0.0, 0.8), (current_window_width() / 2), 0, (current_window_width() / 2), current_window_height());
        draw_text(config.title(), COLOR_WHITE, "font_title", y_offset * 3, x_offset, y_start);
        y_start += y_offset * 3;
        draw_text("Author: " + config.author(), COLOR_WHITE, "font_text", y_offset, x_offset, y_start + (1 * y_offset));
        draw_text("Genre: " + config.genre(), COLOR_WHITE, "font_text", y_offset, x_offset, y_start + (2 * y_offset));
        draw_text("Language: " + config.language(), COLOR_WHITE, "font_text", y_offset, x_offset, y_start + (3 * y_offset));
        draw_text("Rating: " + config.rating(), COLOR_WHITE, "font_text", y_offset, x_offset, y_start + (4 * y_offset));
        draw_text("Repository: " + config.repo(), COLOR_WHITE, "font_text", y_offset, x_offset, y_start + (5 * y_offset));
    }

    // Start up the chosen game using CreateProcessA.
    void start_game(LPCSTR gamePath,LPSTR gameExe, LPCSTR gameDirectory)
    {
        // Additional info
        STARTUPINFOA startupInfo;

        // Set the size of the structures
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        ZeroMemory(&processInfo, sizeof(processInfo));

        // Start the program up
        WINBOOL gameProcess = CreateProcessA
        (
            gamePath,               // the path
            gameExe,                // Command line
            NULL,                   // Process handle not inheritable
            NULL,                   // Thread handle not inheritable
            FALSE,                  // Set handle inheritance to FALSE
            NORMAL_PRIORITY_CLASS,     // Don't open file in a separate console
            NULL,                    // Use parent's environment block
            gameDirectory,           // Use parent's starting directory
            &startupInfo,            // Pointer to STARTUPINFO structure
            &processInfo           // Pointer to PROCESS_INFORMATION structure
        );

        OpenProcess(PROCESS_QUERY_INFORMATION,TRUE, gameProcess);

        this->_in_game = true;
    }

    // Method to keep the mouse positioned within the game window.
    void move_mouse_position(point_2d mousePoint)
    {
        if (this->_in_game == true)
        {
            // Check if game has been exited.
            this->_program_exit = GetExitCodeProcess(processInfo.hProcess, &exit_code);
            if ((this->_program_exit) && (STILL_ACTIVE != exit_code))
            {
                this->_in_game = false;
            }
            // If game is not exited, keep the mouse pointer in the game window.
            int mouse_x = mousePoint.x;
            int mouse_y = mousePoint.y;
            if (mousePoint.x > this->_x || mousePoint.x < this->_x || mousePoint.y > this->_y || mousePoint.y < this->_y)
            {
                move_mouse(_center_x, _center_y);
                this->_x = mouse_x;
                this->_y = mouse_y;
            }
        }
    }

};
