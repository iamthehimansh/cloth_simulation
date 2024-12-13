#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

#include "particle.h"
#include "constraint.h"
#include "input_handler.h"
#include <stdio.h>
#include <stdlib.h>
const int WIDTH = 3840;
const int HEIGHT = 2160;
const float PARTICLE_RADIOUS = 5.f;
const float GRAVITY = 10.0f;
const float TIME_STEP = 0.1f;

const int ROW = 40;
const int COL = 50;
const float REST_DISTANCE = 20.0f;

const float MOUSE_INFLUENCE_RADIUS = 50.0f;
const float MOUSE_FORCE_MULTIPLIER = 1.0f;

const int WIND_CHANGE_INTERVAL = 100;
// float WIND = -3.0f;
Particle* findClosestParticle(const sf::Vector2i& mousePos, std::vector<Particle>& particles) {
    float minDist = MOUSE_INFLUENCE_RADIUS;
    Particle* closest = nullptr;
    
    for (auto& p : particles) {
        float dx = p.position.x - mousePos.x;
        float dy = p.position.y - mousePos.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        
        if (dist < minDist) {
            minDist = dist;
            closest = &p;
        }
    }
    return closest;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Cloth Simulation");
    window.setFramerateLimit(60);
    float WIND = -1.0f;
    std::vector<Particle> particles;
    std::vector<Constraint> constraints;
    int counter = 0;
    for (int row = 0; row < ROW; row++) {
        for (int col = 0; col < COL; col++) {
            float x = col * REST_DISTANCE +REST_DISTANCE*COL/4;
            float y = row * REST_DISTANCE + 10;
            // printf("%d %d\n",row,col);

            if(row%3==0 && row!=0){
                particles.emplace_back(x, y, false,1.5);
                // printf("Added\n");
            
            }
            else if(row%5==1){
                particles.emplace_back(x, y, false,3.0);
            
            }else if(row%7==1){
                particles.emplace_back(x, y, false,2.0);

            }  else
            
            {

                bool pinned = (row == 0);
                particles.emplace_back(x, y, pinned);
            }
        }
    }
    

    // Initialize constraints
    for (int row = 0; row < ROW; row++) {
        for (int col = 0; col < COL; col++) {
            if (col < COL - 1) {
                // Horizontal constraint
                constraints.emplace_back(&particles[row * COL + col], &particles[row * COL + col + 1]);
            }
            if (row < ROW - 1) {
                // Vertical constraint
                constraints.emplace_back(&particles[row * COL + col], &particles[(row + 1) * COL + col]);
            }
        }
    }

    sf::Vector2i lastMousePos;
    bool isDragging = false;
    Particle* selectedParticle = nullptr;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // handle mouse clicks
            InputHandler::handle_mouse_click(event, particles, constraints);

            // Mouse button pressed
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    selectedParticle = findClosestParticle(mousePos, particles);
                    if (selectedParticle) {
                        isDragging = true;
                        lastMousePos = mousePos;
                        selectedParticle->is_pinned = true;  // Line 91

                    }
                }
            }

            // Mouse button released
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (selectedParticle) {
                        selectedParticle->is_pinned = false; // Line 100
                        selectedParticle = nullptr;
                    }
                    isDragging = false;
                }
            }

            // Mouse moved
            if (event.type == sf::Event::MouseMoved) {
                if (isDragging && selectedParticle) {
                    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
                    selectedParticle->position.x += MOUSE_FORCE_MULTIPLIER*(currentMousePos.x - lastMousePos.x);
                    selectedParticle->position.y += MOUSE_FORCE_MULTIPLIER*(currentMousePos.y - lastMousePos.y);
                    lastMousePos = currentMousePos;
                }
            }
        }

        //apply gravity and update particles
        for (auto& particle : particles) {
            particle.apply_force(sf::Vector2f(WIND, GRAVITY));
            particle.update(TIME_STEP);
            particle.constrain_to_bounds(WIDTH, HEIGHT);
        }

        for (size_t i = 0; i < 5; i++) {
            for (auto& constraint : constraints) {
                constraint.satisfy();
            }
        }
        
        window.clear(sf::Color::Black);


        // Draw particles as balls
        for (const auto& particle : particles) {
            // printf("%d\n",particle.mass);
            if(particle.mass!=1){
                sf::CircleShape circle(PARTICLE_RADIOUS*particle.mass);
                if(particle.mass==4.0){
                    circle.setFillColor(sf::Color::Green);
                }else if(particle.mass==2.0){
                    circle.setFillColor(sf::Color::Blue);
                }else{
                circle.setFillColor(sf::Color::Red);
                }
                circle.setPosition(particle.position.x - PARTICLE_RADIOUS, 
                                    particle.position.y - PARTICLE_RADIOUS);
                window.draw(circle);
            }else{

                sf::CircleShape circle(PARTICLE_RADIOUS);
                circle.setFillColor(sf::Color::White);
                circle.setPosition(particle.position.x - PARTICLE_RADIOUS, 
                                    particle.position.y - PARTICLE_RADIOUS);
                window.draw(circle);
            }

        }
        // scanf("%d");

        // Draw particles as points
        // for (const auto& particle : particles) {
        //     sf::Vertex point(particle.position, sf::Color::White);
        //     window.draw(&point, 1, sf::Points);
        // }


        // Draw constraints as lines
        for (const auto& constraint : constraints) {
            if (!constraint.active) {
                continue;
            }
            sf::Vertex line[] = {
                sf::Vertex(constraint.p1->position, sf::Color::Cyan),
                sf::Vertex(constraint.p2->position, sf::Color::Cyan),
            };
            window.draw(line, 2, sf::Lines);
        }

        window.display();
        if(counter++==WIND_CHANGE_INTERVAL){
        WIND=(rand()%10-5)*((rand() % 2) ==0 ?-1:1);
        counter=0;
        }
    }
}