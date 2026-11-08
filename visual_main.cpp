#include "PCBView.h"
#include <iostream>
int main() {
    try {
        PCBView view;
        sf::RenderWindow window(sf::VideoMode(PCBView::WIDTH, PCBView::HEIGHT), "PCB Path Finder", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);
        while (window.isOpen()) {
            sf::Event event{};
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close();
                else view.handle(event, window);
            }
            if (window.isOpen()) { view.draw(window); window.display(); }
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}