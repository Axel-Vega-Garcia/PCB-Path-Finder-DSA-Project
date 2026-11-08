#include "PCBView.h"
#include <cmath>
#include <iomanip>
#include <sstream>
namespace {
    const sf::Color background(12, 20, 23);
    const sf::Color mint(77, 227, 192);
    const sf::Color copper(244, 182, 94), boardGreen(14, 74, 55);
    void rect(sf::RenderTarget& target, sf::FloatRect box, sf::Color fill) {
        sf::RectangleShape shape({box.width, box.height});
        shape.setPosition(box.left, box.top);
        shape.setFillColor(fill);
        target.draw(shape);
    }
}

void PCBView::text(sf::RenderTarget& target, const std::string& str, float x, float y, float size, bool centered, sf::Color color) const {
    sf::Text label(str, font, 48);
    const auto bounds = label.getLocalBounds();
    if (centered) label.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
    label.setPosition(x, y);
    label.setScale(size / 48, size / 48);
    label.setFillColor(color);
    target.draw(label);
}

void PCBView::refresh() {
    sf::Image image;
    image.create(model.grid.width(), model.grid.height(), boardGreen);
    blocked = 0;
    for (int y = 0; y < model.grid.height(); ++y)
        for (int x = 0; x < model.grid.width(); ++x) {
            const Cell& cell = model.grid.at(x, y);
            blocked += cell.isBlocked;
            sf::Color color = boardGreen;
            if (!model.boardCell({x, y})) color = sf::Color::Transparent;
            else if (cell.isBlocked) color = sf::Color(46, 59, 48);
            else if (cell.movementCost > 1) color = sf::Color(91, 70, 42);
            image.setPixel(x, y, color);
        }
    if (!boardTexture.loadFromImage(image)) throw std::runtime_error("Could not create the board image.");
}

void PCBView::fit() {
    float scale = std::min((canvas.width - 96.f) / model.grid.width(), (canvas.height - 80.f) / model.grid.height());
    baseSize = {canvas.width / scale, canvas.height / scale};
    boardView.setCenter(model.grid.width() / 2.f, model.grid.height() / 2.f);
    boardView.setSize(baseSize);
    zoom = 1;
}

void PCBView::path(sf::RenderTarget& target, const PathResult& route, sf::Color color, float offset) const {
    const float scale = canvas.width / boardView.getSize().x;
    const float halfWidth = 1.3f / scale;
    sf::VertexArray lines(sf::Quads);
    for (std::size_t i = 1; i < route.path.size(); ++i) {
        sf::Vector2f a(route.path[i - 1].first + .5f + offset / scale, route.path[i - 1].second + .5f + offset / scale);
        sf::Vector2f b(route.path[i].first + .5f + offset / scale, route.path[i].second + .5f + offset / scale);
        sf::Vector2f normal = a.x == b.x ? sf::Vector2f(halfWidth, 0) : sf::Vector2f(0, halfWidth);
        for (auto p : {a - normal, b - normal, b + normal, a + normal}) lines.append({p, color});
    }
    target.draw(lines);
}

void PCBView::board(sf::RenderTarget& target) const {
    target.setView(boardView);
    const float w = static_cast<float>(model.grid.width()), h = static_cast<float>(model.grid.height());
    target.draw(sf::Sprite(boardTexture));
    sf::VertexArray lines(sf::Lines);
    int stride = canvas.width / boardView.getSize().x < 6 ? 10 : 1;
    for (int x = 0; x <= model.grid.width(); x += stride) {
        lines.append({{static_cast<float>(x), 0}, sf::Color(157, 197, 164, 35)});
        lines.append({{static_cast<float>(x), h}, sf::Color(157, 197, 164, 35)});
    }
    for (int y = 0; y <= model.grid.height(); y += stride) {
        lines.append({{0, static_cast<float>(y)}, sf::Color(157, 197, 164, 35)});
        lines.append({{w, static_cast<float>(y)}, sf::Color(157, 197, 164, 35)});
    }
    target.draw(lines);
    for (const auto& part : model.components) {
        rect(target, {static_cast<float>(part.x), static_cast<float>(part.y), static_cast<float>(part.w), static_cast<float>(part.h)}, sf::Color::Black);
        text(target, part.name, part.x + part.w / 2.f, part.y + part.h / 2.f, std::min({5.5f, part.h * .45f, part.w / static_cast<float>(part.name.size())}), true);
    }
    if (model.hasDijkstra) path(target, model.dijkstraResult, copper, model.hasAStar ? -1.1f : 0);
    if (model.hasAStar) path(target, model.astarResult, mint, model.hasDijkstra ? 1.1f : 0);
    const float marker = std::max(.18f, 7.f * boardView.getSize().x / canvas.width);
    for (int endpoint = 0; endpoint < (model.start == model.end ? 1 : 2); ++endpoint) {
        Position p = endpoint ? model.end : model.start;
        sf::Vector2f center(p.first + .5f, p.second + .5f);
        const sf::Color color = endpoint ? copper : mint;
        sf::CircleShape dot(marker);
        dot.setPosition(center.x - marker, center.y - marker);
        dot.setFillColor(background);
        dot.setOutlineColor(color);
        dot.setOutlineThickness(marker / 5);
        target.draw(dot);
        text(target, model.start == model.end ? "B" : endpoint ? "E" : "S", center.x, center.y, marker * 1.35f, true, color);
    }
}

PCBView::PCBView() {
    if (!font.loadFromFile("C:/Windows/Fonts/consola.ttf")) throw std::runtime_error("Could not load Consolas from C:/Windows/Fonts/consola.ttf");
    boardView.setViewport({canvas.left / WIDTH, canvas.top / HEIGHT, canvas.width / WIDTH, canvas.height / HEIGHT});
    fit(); refresh(); model.run(3);
}

void PCBView::choose(int row) {
    if (row < 6) tool = static_cast<Tool>(row);
    else if (row < 9) model.run(row - 5);
    else if (row == 9) model.preset();
    else if (row == 10) model.clear();
    else if (row == 11) fit();
    if (row == 9 || row == 10) refresh();
}

void PCBView::command() {
    std::istringstream stream(input);
    std::string name, extra;
    stream >> name;
    if (name == "size" || name == "chip") {
        int w, h; char x;
        if (!(stream >> w >> x >> h) || (x != 'x' && x != 'X') || (stream >> extra)) throw std::invalid_argument("Use size 40x30 or chip 20x20");
        if (name == "size") {
            model.resize(w, h);
            chipWidth = std::min(chipWidth, w);
            chipHeight = std::min(chipHeight, h);
            fit();
        } else {
            if (w < 1 || h < 1 || w > model.grid.width() || h > model.grid.height()) throw std::invalid_argument("Chip dimensions must be positive and fit the board.");
            chipWidth = w; chipHeight = h; tool = Tool::Chip;
            model.status = "Click empty space to place a chip, or click a chip to resize it.";
        }
    } else if (name == "random") {
        double percent; long long seed;
        if (!(stream >> percent >> seed) || (stream >> extra) || seed < 0 || seed > 4294967295LL) throw std::invalid_argument("Use random 15 42 (percent, seed)");
        model.randomize(percent, static_cast<std::uint32_t>(seed));
    } else if (name == "cost" || name == "brush") {
        int n;
        const bool weight = name == "cost";
        if (!(stream >> n) || (stream >> extra) || n < (weight ? 1 : 0) || n > (weight ? 100 : 20)) throw std::invalid_argument("Cost: 1-100. Brush radius: 0-20.");
        if (weight) cost = n;
        else radius = n;
        model.status = name + " set to " + std::to_string(n);
    } else throw std::invalid_argument("Commands: size 40x30 | chip 20x20 | random 15 42 | cost 5 | brush 3");
    input.clear();
    refresh();
}

bool PCBView::edit(Position p) {
    if (tool == Tool::Start || tool == Tool::End) return model.select(tool, p);
    if (tool == Tool::Chip) return model.chip(p, chipWidth, chipHeight);
    return model.brush(tool, p, radius, cost);
}

void PCBView::handle(const sf::Event& e, const sf::RenderTarget& target) {
    try {
        if (e.type == sf::Event::LostFocus) cancelDrag();
        if (e.type == sf::Event::TextEntered) {
            if (e.text.unicode == 8 && !input.empty()) input.pop_back();
            else if (e.text.unicode >= 32 && e.text.unicode < 127 && input.size() < 48) input += static_cast<char>(e.text.unicode);
        }
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Escape) input.clear();
            if (e.key.code == sf::Keyboard::Enter && !input.empty()) command();
        }
        if (e.type == sf::Event::MouseWheelScrolled) {
            sf::Vector2i p{e.mouseWheelScroll.x, e.mouseWheelScroll.y};
            if (canvas.contains(sf::Vector2f(p))) {
                const auto before = target.mapPixelToCoords(p, boardView);
                zoom = std::clamp(zoom * std::pow(1.2f, e.mouseWheelScroll.delta), .65f, 24.f);
                boardView.setSize(baseSize / zoom);
                boardView.move(before - target.mapPixelToCoords(p, boardView));
            }
        }
        const bool pressed = e.type == sf::Event::MouseButtonPressed;
        const bool released = e.type == sf::Event::MouseButtonReleased;
        const bool moved = e.type == sf::Event::MouseMoved;
        if (!pressed && !released && !moved) return;
        sf::Vector2i pixel = moved ? sf::Vector2i(e.mouseMove.x, e.mouseMove.y) : sf::Vector2i(e.mouseButton.x, e.mouseButton.y);
        const auto delta = pixel - pressMouse;
        const bool movedFar = std::abs(delta.x) > 4 || std::abs(delta.y) > 4;
        if (released) {
            if (e.mouseButton.button == sf::Mouse::Right) {
                if (dragging == sf::Mouse::Right) cancelDrag(); 
                return;
            }
            if (e.mouseButton.button != sf::Mouse::Left || dragging != sf::Mouse::Left) return;
            dragging.reset();
            if (dragged || movedFar || !canvas.contains(sf::Vector2f(pixel))) return;
            pixel = pressMouse;
        } else if (pressed) {
            if (e.mouseButton.button == sf::Mouse::Left) {
                cancelDrag();
                if (pixel.x >= 1120 && pixel.x < 1420 && pixel.y >= 145 && pixel.y < 145 + 12 * 28) choose((pixel.y - 145) / 28);
                else if (canvas.contains(sf::Vector2f(pixel))) {dragging = sf::Mouse::Left; dragged = false; pressMouse = lastMouse = pixel;}
                return;
            }
            if (e.mouseButton.button != sf::Mouse::Right || dragging == sf::Mouse::Left || (tool != Tool::Block && tool != Tool::Erase && tool != Tool::Weight) || !canvas.contains(sf::Vector2f(pixel))) return;
            dragging = sf::Mouse::Right;
        } else if (dragging == sf::Mouse::Left) {
            if (!dragged && !movedFar) return;
            dragged = true;
            boardView.move(target.mapPixelToCoords(lastMouse, boardView) - target.mapPixelToCoords(pixel, boardView));
            lastMouse = pixel;
            return;
        } else if (dragging != sf::Mouse::Right) return;
        const auto world = target.mapPixelToCoords(pixel, boardView);
        Position p{static_cast<int>(std::floor(world.x)), static_cast<int>(std::floor(world.y))};
        if (!canvas.contains(sf::Vector2f(pixel)) || !model.grid.inBounds(p.first, p.second)) {lastPaint.reset(); return;}
        const Position from = moved ? lastPaint.value_or(p) : p;
        const int steps = std::max(std::abs(p.first - from.first), std::abs(p.second - from.second));
        bool changed = false;
        for (int i = 0; i <= steps; ++i) {
            const float t = steps ? static_cast<float>(i) / steps : 0;
            changed |= edit({static_cast<int>(std::lround(from.first + t * (p.first - from.first))), static_cast<int>(std::lround(from.second + t * (p.second - from.second)))});
        }
        if (changed) refresh();
        lastPaint = p;
    } catch (const std::exception& error) { cancelDrag(); model.status = error.what(); }
}

void PCBView::draw(sf::RenderTarget& target) const {
    target.clear(background);
    target.setView(target.getDefaultView());
    rect(target, canvas, sf::Color(15, 26, 29));
    board(target);
    target.setView(target.getDefaultView());
    text(target, "PCB Path Finder", 24, 24);
    text(target, std::to_string(model.grid.width()) + " x " + std::to_string(model.grid.height()) + " | Available: " + std::to_string(model.grid.size() - blocked), 24, 66);
    text(target, "Click a menu item:", 1120, 112);
    const char* menu[] = {"Set start", "Set end", "Paint obstacle", "Erase", "Paint cost", "Place / resize chip", "Run Dijkstra", "Run A*", "Compare both", "PCB preset", "Clear board", "Fit view"};
    for (int row = 0; row < 12; ++row) text(target, std::string(row == static_cast<int>(tool) ? "> " : "  ") + menu[row], 1120, 145.f + row * 28);
    std::ostringstream info;
    info << "Start: (" << model.start.first << ", " << model.start.second << ")\nEnd: ("
         << model.end.first << ", " << model.end.second << ")\nCost: " << cost << " | Brush: " << radius
         << "\nChip: " << chipWidth << "x" << chipHeight << "\n";
    for (int i = 0; i < 2; ++i) {
        const bool ran = i ? model.hasAStar : model.hasDijkstra;
        const auto& r = i ? model.astarResult : model.dijkstraResult;
        info << (i ? "A* (mint)" : "Dijkstra (copper)") << ": ";
        if (!ran) info << "not run\n";
        else info << (r.found ? std::to_string(r.path.size() - 1) + " moves" : "no route") << "\n"
                  << "Cost " << r.totalCost << " | " << r.nodesExplored << " explored\n"
                  << std::fixed << std::setprecision(2) << r.timeMs << " ms\n";
    }
    text(target, info.str(), 1120, 520);
    text(target, "Type a command, then Enter:", 1120, 728);
    text(target, "> " + input.substr(input.size() > 32 ? input.size() - 32 : 0) + "|", 1120, 752);
    text(target, "Example command:\nsize 40x30\nchip 20x20\nrandom 15 42\ncost 5\nbrush 3", 1120, 790);
    text(target, "Wheel: zoom | Left drag: pan | Left click: use tool | Right drag: paint | Esc: clear command\n"
                 "Chip size: type chip 20x20 in command, press Enter, then click a chip or empty space.", 24, 900);
    text(target, model.status, 24, 940);
}