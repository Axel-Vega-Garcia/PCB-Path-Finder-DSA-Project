#pragma once
#include "BoardModel.h"
#include <SFML/Graphics.hpp>
#include <optional>

class PCBView {
    
public:
    static constexpr unsigned WIDTH = 1440, HEIGHT = 960;
    PCBView();
    void handle(const sf::Event& event, const sf::RenderTarget& target);
    void draw(sf::RenderTarget& target) const;
    const BoardModel& state() const { return model; }
    sf::Vector2i cellPixel(Position p, const sf::RenderTarget& target) const {return target.mapCoordsToPixel({p.first + .5f, p.second + .5f}, boardView);}

private:
    BoardModel model;
    Tool tool = Tool::Start;
    sf::Font font;
    sf::Texture boardTexture;
    sf::View boardView;
    const sf::FloatRect canvas{24, 112, 1066, 776};
    sf::Vector2f baseSize;
    sf::Vector2i lastMouse, pressMouse;
    std::optional<sf::Mouse::Button> dragging;
    std::optional<Position> lastPaint;
    std::string input;
    
    int radius = 3, cost = 5, chipWidth = 20, chipHeight = 20;
    bool dragged = false;
    float zoom = 1;
    std::size_t blocked = 0;
    void refresh();
    void cancelDrag() {dragging.reset(); lastPaint.reset();}
    void fit();
    void command();
    void choose(int row);
    bool edit(Position p);
    void board(sf::RenderTarget& target) const;
    void path(sf::RenderTarget& target, const PathResult& route, sf::Color color, float offset) const;
    void text(sf::RenderTarget& target, const std::string& value, float x, float y, float size = 14, bool centered = false, sf::Color color = sf::Color::White) const;
};