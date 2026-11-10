#include "raylib.h"
#include "Game.h"

int main()
{ 
    game TpfinalMavi;

    TpfinalMavi.Inits();
    while (!WindowShouldClose())
    {
        BeginDrawing();

        bool shouldContinue = TpfinalMavi.Run();

        EndDrawing();

        if (!shouldContinue) {
            break;
        }
    }

    return 0;
}

