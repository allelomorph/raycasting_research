#ifndef APP_HH
#define APP_HH

#include "State.hh"
//#include "../include/Matrix.hh"  // Vector2d Vector2i

// console chars, not pixels
#define WINDOW_WIDTH 108
#define WINDOW_HEIGHT 64

// used Eigen::Vector2d, Eigen::Vector2i
class App {
public:
    App();
    ~App();
    void run(std::string filename);
private:
    State *state;
    void initialize(std::string filename);
    /*
    void drawTexture(
        int x, int side, int lineheight, double perpWallDist,
        int drawstart, int drawend,
        Eigen::Vector2d& ray, Eigen::Vector2i& mapPos);  // helper to drawLine
    void drawLine(int x);                                // helper to render3d
    void render3d();
    // void renderMap();
    void displayFPS(double fps);
    */
    void getEvents();
    /*
    // Vector2d rotate2d(Vector2d vector, double rotSpeed)  // helper to updateData
    void updateData(double frameTime);
    */
};

#endif  // APP_HH
