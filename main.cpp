#include "include/raylib-cpp.hpp"
#include <iostream>
#include <algorithm>
#include <vector>

const int gridRows = 10;
const int gridColumns = 10;
const int maxCells = gridRows * gridColumns;
const int gridThickness = 2;
const int gridSize = 25;
const int gridBorder = 10;
const int screenWidth = gridColumns * gridSize + (gridColumns + 1 ) * gridThickness + gridBorder*2;
const int screenHeight = gridRows * gridSize + (gridRows + 1) * gridThickness + gridBorder*2;

const int splashScreenGridSize = 10;
const int splashScreenGridCols = 16;
const int splashScreenGridRows = 16;
const int splashScreenGridCount = 2 * splashScreenGridCols + 2 * splashScreenGridRows - 4;

const raylib::Color gridColor = {218, 223, 225, 255};
const raylib::Color foodColor = {255, 0, 0, 255};
const raylib::Color snakeHeadColorStart = {81, 134, 236, 255};
const raylib::Color snakeHeadColorEnd = {200, 200, 200, 255};

const raylib::Color foreground = {0, 68, 130};
const raylib::Color background = RAYWHITE;
const raylib::Color gray = GRAY;

const int startingSpeed = 22;
const int maxSpeed = 5;

const raylib::Vector2 Up = {0, -1};
const raylib::Vector2 Down = {0, 1};
const raylib::Vector2 Left = {-1, 0};
const raylib::Vector2 Right = {1, 0};

enum GameState {
    SPLASH_SCREEN,
    GAME,
    GAME_OVER
};

struct Game {
    std::vector<raylib::Vector2> Snake;
    raylib::Vector2 Direction;
    raylib::Vector2 LastInput;
    raylib::Vector2 Food;
    int Speed;
    int Ticks;
    GameState State;
};


void InitializeGame(Game & game);

/* Splash Screen*/

void ReadInputsSplashScreen(Game & game) {
    if(IsKeyPressed(KEY_SPACE)){
        InitializeGame(game);
    }
}

/*
Calculates the position of the ouroboros on the logo in "grid" coordinates
 positition 0
    x x x x x
    x       x
    x       x
    x       x
    x x x x x

    Returns a Vector2 with {cols, rows} of the position
*/

raylib::Vector2 GetOuroborosPosition(int i) {
    if(i < splashScreenGridCols) {
        return {static_cast<float>(i), 0.0f};
    } else if(i < splashScreenGridCols + splashScreenGridRows - 1) {
        return {static_cast<float>(splashScreenGridCols - 1), static_cast<float>(i - splashScreenGridCols + 1)};
    } else if(i < 2 * splashScreenGridCols + splashScreenGridRows - 2) {
        return {static_cast<float>(2 * splashScreenGridCols + splashScreenGridRows - 3 - i), static_cast<float>(splashScreenGridRows - 1)};
    } else {
        return {0.0f, static_cast<float>(2 * splashScreenGridCols + 2 * splashScreenGridRows - 4 - i)};
    }
}

void DrawSplashScreen(raylib::Window & window, Game & game) {
    BeginDrawing();
    window.ClearBackground(background);

    int upperLeftX = screenWidth/2 - splashScreenGridCols/2 * splashScreenGridSize;
    int upperLeftY = screenHeight/2 - splashScreenGridRows/2 * splashScreenGridSize;

    foreground.DrawRectangle(
        upperLeftX,
        upperLeftY,
        splashScreenGridCols * splashScreenGridSize,
        splashScreenGridRows * splashScreenGridSize
    );

    background.DrawRectangle(
        screenWidth/2 - splashScreenGridCols/2 * splashScreenGridSize + splashScreenGridSize,
        screenHeight/2 - splashScreenGridRows/2 * splashScreenGridSize + splashScreenGridSize,
        (splashScreenGridCols-2) * splashScreenGridSize,
        (splashScreenGridCols-2) * splashScreenGridSize 
    );

    // This draws an ouroboros on the logo
    for(int i = 0; i < 6; i ++) {
        int snakeOffset = (game.Ticks / 5 + i) % splashScreenGridCount;
        raylib::Vector2 gridPosition = GetOuroborosPosition(snakeOffset);
        background.DrawRectangle(
            upperLeftX + gridPosition.x * splashScreenGridSize + 1,
            upperLeftY + gridPosition.y * splashScreenGridSize + 1,
            splashScreenGridSize - 2,
            splashScreenGridSize - 2
        );
    }

    std::string start_text = "Press Space to Start";
    int textWidth = MeasureText(start_text.c_str(), 20);

    foreground.DrawText("snek", screenWidth * 0.460, screenHeight *5.0/8, 30);

    gray.DrawText(start_text, screenWidth/2 - textWidth/2, screenHeight * 7.0/8, 20);

    EndDrawing();
}

void UpdateSplashScreenState(Game & game) {
    game.Ticks++;
}

void SplashScreen(raylib::Window & window, Game & gameState) {
    ReadInputsSplashScreen(gameState);
    UpdateSplashScreenState(gameState);
    DrawSplashScreen(window, gameState);
}

/* Game Screen */

void DrawGrid() {
    // Columns
    for(int col = 0; col <= gridColumns; col++){
        DrawRectangle(gridBorder + col *(gridSize+gridThickness), gridBorder, gridThickness, screenHeight-gridBorder*2, gridColor);
    }

    // Rows
    for(int row = 0; row <= gridRows; row++){
        DrawRectangle(gridBorder, gridBorder + row *(gridSize+gridThickness), screenWidth-gridBorder*2, gridThickness, gridColor);
    }
}

void DrawBox(raylib::Vector2 position, Color color){
    DrawRectangle(
        gridBorder + gridThickness + position.x * (gridSize + gridThickness),
        gridBorder + gridThickness + position.y * (gridSize + gridThickness),
        gridSize,
        gridSize,
        color
    );
}

void DrawSnake(Game & game){
    for(size_t i = 0; i < game.Snake.size(); i++) {
        Color segmentColor = ColorLerp(snakeHeadColorStart, snakeHeadColorEnd, 1.0f * i / maxCells);
        DrawBox(game.Snake[i], segmentColor);
    }
}

void DrawGameScreen(raylib::Window & window, Game & game){
    BeginDrawing();
    window.ClearBackground(background);
    DrawGrid();
    DrawSnake(game);
    DrawBox(game.Food, foodColor);
    EndDrawing();
}

void ReadGameInputs(Game & game) {
    if(IsKeyPressed(KEY_UP)){
        game.LastInput = Up;
    } else if (IsKeyPressed(KEY_DOWN) && game.Direction != Up){
        game.LastInput = Down;
    } else if (IsKeyPressed(KEY_LEFT) && game.Direction != Right){
        game.LastInput = Left;
    } else if (IsKeyPressed(KEY_RIGHT) && game.Direction != Left){
        game.LastInput = Right;
    }
}

bool CheckCollision(raylib::Vector2 & position, Game & game) {
    auto it = std::find_if(
        game.Snake.begin(),
        game.Snake.end(),
        [position](raylib::Vector2 & segment){ return segment.x == position.x && segment.y == position.y; }
    );

    return it != game.Snake.end();
}

void PlaceFood(Game & game) {
    while(true) {
        float col = static_cast<float>(rand() % gridColumns);
        float row = static_cast<float>(rand() % gridRows);

        raylib::Vector2 foodPosition = {col, row};

        if(!CheckCollision(foodPosition, game)){
            game.Food = foodPosition;
            return;
        }
    }
}

void UpdateGameState(Game & game) {
    game.Ticks++;

    if(game.Ticks < game.Speed) { return; }

    if(game.LastInput == Left && game.Direction != Right){
        game.Direction = Left;
    } else if (game.LastInput == Right && game.Direction != Left){
        game.Direction = Right;
    } else if (game.LastInput == Up && game.Direction != Down){
        game.Direction = Up;
    } else if (game.LastInput == Down && game.Direction != Up){
        game.Direction = Down;
    }

    game.Ticks = 0;
    raylib::Vector2 nextPosition = game.Snake[0] + game.Direction;

    if(nextPosition == game.Food){
        game.Snake.insert(game.Snake.begin(), nextPosition);
        PlaceFood(game);
        game.Speed = startingSpeed - (game.Snake.size() * 1.0f)/maxCells *(startingSpeed - maxSpeed);
    } else if (nextPosition.x < 0 || nextPosition.x >= gridColumns || nextPosition.y < 0 || nextPosition.y >= gridRows || CheckCollision(nextPosition, game)){
        game.State = GAME_OVER;
    }
    else {
        game.Snake.insert(game.Snake.begin(), nextPosition);
        game.Snake.pop_back();
    }
}


void GameScreen(raylib::Window & window, Game & gameState) {
    ReadGameInputs(gameState);
    DrawGameScreen(window, gameState);
    UpdateGameState(gameState);
}

/* Game Over */
void DrawGameOverScreen(Game & game) {
    BeginDrawing();
    ClearBackground(background);
    std::string game_over_text = "Game Over";
    int gameOverTextWidth = MeasureText(game_over_text.c_str(), 45);
    gray.DrawText(game_over_text, screenWidth/2 - gameOverTextWidth/2, screenHeight * 1.0/8, 45);

    std::string score_text = "Your Score: " + std::to_string(game.Snake.size() - 1);
    int scoreTextWidth = MeasureText(score_text.c_str(), 20);
    gray.DrawText(score_text, screenWidth/2 - scoreTextWidth/2, screenHeight * 3.0/8, 20);

    std::string start_text = "Press Space to Restart";
    int restartTextWidth = MeasureText(start_text.c_str(), 20);
    gray.DrawText(start_text, screenWidth/2 - restartTextWidth/2, screenHeight * 7.0/8, 20);
    EndDrawing();
}

void ReadGameOverInputs(Game & game) {
    if(IsKeyPressed(KEY_SPACE)){
        game.State = SPLASH_SCREEN;
    }
}

void GameOverScreen(raylib::Window & window, Game & gameState) {
    ReadGameOverInputs(gameState);
    DrawGameOverScreen(gameState);
}

/* Utility */
void InitializeGame(Game & game) {
    game.Snake.clear();
    game.Snake.push_back({gridColumns/2, gridRows/2});
    game.Direction = raylib::Vector2(1, 0);
    game.Speed = startingSpeed;
    game.Ticks = 0;
    game.State = GAME;
    PlaceFood(game);
}

int main()
{
    raylib::Window window(screenWidth, screenHeight, "Snek");
    SetTargetFPS(60);

    Game gameState;
    gameState.State = SPLASH_SCREEN;

    while(!window.ShouldClose()) {
        if(gameState.State == SPLASH_SCREEN){
            SplashScreen(window, gameState);
        } else if(gameState.State == GAME){
            GameScreen(window, gameState);
        } else if(gameState.State == GAME_OVER){
            GameOverScreen(window, gameState);
        }
    }

    CloseWindow();
}
