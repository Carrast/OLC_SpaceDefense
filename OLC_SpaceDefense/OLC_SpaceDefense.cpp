// OLC_SpaceDefense.cpp
// written by Carrast
// 2022.09.07
// all rights reserved
// thanks javidx9!

#include <iostream>
#include <string>
#include <math.h>
#include "olcConsoleGameEngine.h"

class SpaceDefense : public olcConsoleGameEngine
{
public:
    SpaceDefense();
    ~SpaceDefense() {}

private:
    struct sStars
    {
        float x;
        float y;
        float dx;
        float dy;
    };

    struct sBullets
    {
        float x;
        float y;
        float dx;
        float dy;
        int damage;
    };

    struct sObjects
    {
        float x;
        float y;
        float dx;
        float dy;
        float angle;
        float scale;
        int health;
        int bounce;
    };

    std::vector<sStars> vecStars;
    sObjects tank;
    std::vector<sBullets> vecBullets;
    std::vector<sObjects> vecMeteors;
    std::vector<sObjects> vecDebris;

    std::vector<std::pair<float, float>>  vecModelTank;
    std::vector<std::pair<float, float>>  vecModelMeteors;
    std::vector<std::pair<float, float>>  vecModelDebris;

    float speedBoost = 0.0f;
    int groundLevel = 95;
    int baseHealth = 100;
    int nScore;
    bool bBaseBlownUp = false;

    // time
    //std::chrono::duration<double> durTimeOnGround(0);

protected:
    // Called by olcConsoleGameEngine
    virtual bool OnUserCreate()
    {
        // background stars
        vecStars.push_back({ 32.0f, 5.0f, -4.5f, 0.0f });
        vecStars.push_back({ 2.0f, 10.0f, -4.5f, 0.0f });
        vecStars.push_back({ 150.0f, 12.0f, -4.5f, 0.0f });
        vecStars.push_back({ 22.0f, 23.0f, -4.5f, 0.0f });
        vecStars.push_back({ 84.0f, 32.0f, -4.5f, 0.0f });
        vecStars.push_back({ 42.0f, 45.0f, -4.5f, 0.0f });
        vecStars.push_back({ 131.0f, 49.0f, -4.5f, 0.0f });
        vecStars.push_back({ 30.0f, 56.0f, -4.5f, 0.0f });
        vecStars.push_back({ 122.0f, 87.0f, -4.5f, 0.0f });
        vecStars.push_back({ 15.0f, 92.0f, -4.5f, 0.0f });
        vecStars.push_back({ 62.0f, 70.0f, -4.5f, 0.0f });

        // tank model
        vecModelTank =
        {
            { 0.0f, -5.0f},
            { -2.5f, 0.0f},
            { 2.5f, 0.0f}
        };

        // meteor model
        int numSections = 20;
        for (int i = 0; i < numSections; i++)
        {
            float radius = ((float)std::rand() / (float)RAND_MAX) * 0.4f + 0.7f;
            float angle = ((float)i / (float)numSections) * 2.0f * 3.14159f;
            vecModelMeteors.push_back(std::make_pair(radius * cosf(angle), radius * sinf(angle)));
        }

        // debris model
        numSections = 5;
        for (int i = 0; i < numSections; i++)
        {
            float radius = ((float)std::rand() / (float)RAND_MAX) * 0.3f + 0.2f;
            float angle = ((float)i / (float)numSections) * 2.0f * 3.14159f;
            vecModelDebris.push_back(std::make_pair(radius * cosf(angle), radius * sinf(angle)));
        }

        ResetGame();
        return true;
    }

    // Called by olcConsoleGameEngine
    virtual bool OnUserUpdate(float fElapsedTime)
    {
        if (baseHealth <= 0)
        {
            bBaseBlownUp == true;
            ResetGame();
        }

        // clear screen, background fill, reset every frame
        Fill(0, 0, m_nScreenWidth, 97, PIXEL_SOLID, 0);

        // background stars
        for (auto& s : vecStars)
        {
            s.x += s.dx * fElapsedTime;
            s.y += s.dy * fElapsedTime;
            WrapCoordinates(s.x, s.y, s.x, s.y);
            Draw(s.x, s.y, PIXEL_SOLID, FG_DARK_GREY);
        }

        // tank movement
        if (m_keys[VK_LSHIFT].bHeld)
            speedBoost = 50.0f;
        else
            speedBoost = 0.0f;
        if (m_keys[VK_LEFT].bHeld)
            tank.x += -(40.0f + speedBoost) * fElapsedTime;
        if (m_keys[VK_RIGHT].bHeld)
            tank.x += +(40.0f + speedBoost) * fElapsedTime;

        // limit tank mobility
        if (tank.x < 3.5)
            tank.x = 3.5;
        if (tank.x > ScreenWidth() - 3.5)
            tank.x = ScreenWidth() - 3.5;

        // fire bullets
        if (m_keys[VK_SPACE].bPressed)
        {
            vecBullets.push_back({ tank.x, tank.y - 5.0f, 0.0f, -55.0f, 5 });
        }

        // update and draw bullets
        for (auto& b : vecBullets)
        {
            //b.x += b.dx * fElapsedTime;
            b.y += b.dy * fElapsedTime;
            Draw(b.x, b.y, PIXEL_SOLID, FG_RED);
        }

        // check collision with meteors
        for (auto& b : vecBullets)
        {
            for (auto& m : vecMeteors)
            {
                if (IsPointInCircle(m.x, m.y, m.scale, b.x, b.y))
                {
                    // meteor hit! remove bullet from gamespace
                    b.x = -100;
                    nScore += 5;
                    Draw(m.x, m.y, PIXEL_SOLID, FG_RED);


                    if (m.health > 0)
                    {
                        m.health -= 1;
                    }
                    else
                    {
                        // meteor blown up
                        nScore += 100;
                        DrawCircle(m.x, m.y, 3, PIXEL_SOLID, FG_RED);

                        // create 2 debris falling from the sky
                        float nx1 = ((float)std::rand() / (float)RAND_MAX) * 20.0f - 10.0f;
                        float ny1 = -5.0f;
                        float nx2 = ((float)std::rand() / (float)RAND_MAX) * 20.0f - 10.0f;
                        float ny2 = -5.0f;
                        vecDebris.push_back({ m.x + 2.0f, m.y + 2.0f, nx1, ny1, 0.0f, 7.0f, 1, 0 });
                        vecDebris.push_back({ m.x - 2.0f, m.y + 2.0f, nx2, ny2, 0.0f, 7.0f, 1, 0 });

                        // remove meteor from gamespace
                        m.x = -100;
                    }
                }
            }
        }

        // check collision with debris
        for (auto& b : vecBullets)
        {
            for (auto& d : vecDebris)
            {
                if (IsPointInCircle(d.x, d.y, d.scale, b.x, b.y))
                {
                    // debris hit! 
                    b.x = -100;
                    nScore += 30;
                    Draw(d.x, d.y, PIXEL_SOLID, FG_RED);

                    // remove debris from gamespace
                    d.x = -100;
                }
            }
        }

        // update and draw meteors
        for (auto& m : vecMeteors)
        {
            m.x += m.dx * fElapsedTime;
            m.y += m.dy * fElapsedTime;

            if (m.y >= groundLevel)
            {
                // remove from gamespace and decrease base health
                m.x = -100;
                baseHealth -= 5;
            }
            m.angle += (((float)std::rand() / (float)RAND_MAX) * 3.0f) * fElapsedTime;
            DrawWireFrameModel(vecModelMeteors, m.x, m.y, m.angle, m.scale, PIXEL_SOLID, FG_YELLOW);
        }

        // update and draw debris
        for (auto& d : vecDebris)
        {
            d.x += d.dx * fElapsedTime;
            if (d.y < groundLevel)
                d.y += ((d.dy + 25.0f)) * fElapsedTime;
            if (d.y >= groundLevel)
            {
                // remove from gamespace and decrease base health
                d.x = -100;
                baseHealth -= 1;
            }

            d.angle += (((float)std::rand() / (float)RAND_MAX) * 3.0f) * fElapsedTime;
            DrawWireFrameModel(vecModelDebris, d.x, d.y, d.angle, d.scale, PIXEL_SOLID, FG_YELLOW);
        }


        // 2 of 3  meteors shot down? get more!
        float rx, ry, rdy;
        if (vecMeteors.size() < 2)
        {
            for (int i = 0; i < 3; i++)
            {
                rx = ((float)std::rand() / (float)RAND_MAX) * 6.0f - 3.0f;
                ry = ((float)std::rand() / (float)RAND_MAX) * 5.0f + 15.0f;
                vecMeteors.push_back({ 38.f + (i * 30), -5.0f, rx, ry, 0.0f, 9.0f, 2 });
            }
        }

        // remove off screen objects
        RemoveObjectsFromGameSpace(vecBullets);
        RemoveObjectsFromGameSpace(vecMeteors);
        RemoveObjectsFromGameSpace(vecDebris);

        // draw tank
        DrawWireFrameModel(vecModelTank, tank.x, tank.y, tank.angle, tank.scale, PIXEL_SOLID, FG_WHITE);

        // monitoring
        DrawString(2, 2, L"baseHealth: " + std::to_wstring(baseHealth));
        DrawString(2, 3, L"nBullets: " + std::to_wstring(vecBullets.size()));
        DrawString(60, 2, L"nScore: " + std::to_wstring(nScore));

        // draw ground
        Fill(0, 97, m_nScreenWidth, m_nScreenHeight, PIXEL_SOLID, FG_GREY);

        return true;
    }

    // --- special functions ---
protected:
    template <class T>
    void RemoveObjectsFromGameSpace(std::vector<T>& vecObjects)
    {
        if (vecObjects.size() > 0)
        {
            auto i = std::remove_if(vecObjects.begin(), vecObjects.end(), [&](T o) { return (o.x < -15 || o.y < -15 || o.x > ScreenWidth() + 15); });
            if (i != vecObjects.end())
            {
                vecObjects.erase(i);
            }
        }
    }

    void ResetGame()
    {
        nScore = 0;
        baseHealth = 100;
        bBaseBlownUp = false;

        vecBullets.clear();
        vecMeteors.clear();
        vecDebris.clear();

        // initiate tank position
        tank.x = ScreenWidth() / 2;
        tank.y = groundLevel;
        tank.dx = 0;
        tank.dy = 0;
        tank.angle = 0;
        tank.scale = 1.0f;
        tank.health = 100;

        // initiate 3 meteors at game start
        float rx, ry, rdy;
        for (int i = 0; i < 3; i++)
        {
            rx = ((float)std::rand() / (float)RAND_MAX) * 6.0f - 3.0f;
            ry = ((float)std::rand() / (float)RAND_MAX) * 2.0f + 5.0f;
            vecMeteors.push_back({ 38.f + (i * 30), -5.0f, rx, ry, 0.0f, 9.0f, 2 });
        }
    }

    bool IsPointInCircle(float cx, float cy, float r, float x, float y)
    {
        float distance = std::sqrtf( (cx - x) * (cx - x) + (cy - y) * (cy - y));
        if (distance <= r)
            return true;
        else
            return false;
    }
    
    void WrapCoordinates(float ix, float iy, float& ox, float& oy)
    {
        ox = ix;
        oy = iy;
        if (ix < 0)
            ox = ix + ScreenWidth();
        if (ix > ScreenWidth())
            ox = ix - ScreenWidth();
        if (iy < 0)
            oy = iy + ScreenHeight();
        if (iy > ScreenHeight())
            oy = iy - ScreenWidth();
    }

    void DrawWireFrameModel(const std::vector<std::pair<float, float>> &vecModelCoordinates, float x, float y, float a, float s, short c, short col)
    {
        // first = x
        // second = y
        
        std::vector<std::pair<float, float>> TransformedCoordinates;
        int verts = vecModelCoordinates.size();
        TransformedCoordinates.resize(verts);

        // rotate
        for (int i = 0; i < verts; i++)
        {
            TransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(a) - vecModelCoordinates[i].second * sinf(a);
            TransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(a) + vecModelCoordinates[i].second * cosf(a);
        }

        // scale
        for (int i = 0; i < verts; i++)
        {
            TransformedCoordinates[i].first = TransformedCoordinates[i].first * s;
            TransformedCoordinates[i].second = TransformedCoordinates[i].second * s;
        }

        // translate
        for (int i = 0; i < verts; i++)
        {
            TransformedCoordinates[i].first = TransformedCoordinates[i].first + x;
            TransformedCoordinates[i].second = TransformedCoordinates[i].second + y;
        }

        // draw
        for (int i = 0; i < verts + 1; i++)
        {
            int j = i + 1;
            DrawLine(TransformedCoordinates[i % verts].first, TransformedCoordinates[i % verts].second, TransformedCoordinates[j % verts].first, TransformedCoordinates[j % verts].second, c, col);
        }
    }
};

SpaceDefense::SpaceDefense()
{
    m_sAppName = L"Space Defense";
}

int main()
{
    SpaceDefense game;
    game.ConstructConsole(160, 100, 8, 8);
    game.Start();
    return 0;
}

