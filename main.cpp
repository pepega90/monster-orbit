#include <raylib.h>
#include <iostream>
#include <vector>

const int WIDTH = 640;
const int HEIGHT = 480;
int centerX = WIDTH / 2;
int centerY = HEIGHT / 2;

struct Bullet
{
    Vector2 pos;
    Vector2 vel;
    int radius;

    Bullet(float x, float y, int radius)
    {
        this->pos = Vector2(x, y);
        this->radius = radius;
    }
};

struct Player
{
    Vector2 pos;
    Rectangle rect;
    Vector2 acc;
    Vector2 vel;
    std::vector<Bullet> bullets;
    int radius;
    Texture2D img;
    float angle;
    int health;

    Player(float x, float y, Texture2D img)
    {
        this->pos = Vector2(x, y);
        this->img = img;
        this->health = 100;
    };

    void Update(bool &startGame, bool &gameOver, bool &musuhDead)
    {
        if (startGame && !gameOver && !musuhDead)
        {
            if (IsKeyDown(KEY_RIGHT))
            {
                angle += 0.1;
            }
            if (IsKeyDown(KEY_LEFT))
            {
                angle -= 0.1;
            }
        }

        // buat posisi player dengan radius
        pos.x = radius * cos(angle) + WIDTH / 2 - 30;
        pos.y = radius * sin(angle) + HEIGHT / 2 - 30;

        rect.x = pos.x;
        rect.y = pos.y;
        rect.width = img.width - 15;
        rect.height = img.height - 15;
        // draw rectangle collision
        // DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, YELLOW);
    }
};

struct Enemy
{
    Vector2 pos;
    std::vector<Texture2D> img;
    std::vector<Bullet> bullets;
    int currentFrame = 0;
    float lastUpdate = 0.2;
    float runAnimateTime;
    bool hit;
    int countHit = 2;
    int countShot;
    int health;
    int maxHealth = 100;
    int minHealth = 0;
    int minSpeed = 5;
    int maxSpeed = 15;
    int minDamage = 10;
    int maxDamage = 20;
    int speed;
    int damage;
    float lastHit = GetTime();
    float lastShoot = GetTime();

    Enemy(float x, float y)
    {
        this->pos = Vector2(x, y);
        this->health = 100;
        this->hit = false;
        this->countShot = 2;
    }

    void Animate(float dt)
    {
        runAnimateTime += dt;
        if (runAnimateTime >= lastUpdate)
        {
            currentFrame++;
            if (currentFrame > img.size() - 1)
                currentFrame = 0;

            runAnimateTime = 0.0;
        }
    }

    void Update(float dt)
    {

        if (hit)
        {
            if (GetTime() - lastHit > 0.3)
            {
                countHit -= 1;
                lastHit = GetTime();
            }
        }

        if (countHit == 0)
        {
            hit = false;
            if (health > 0)
                health -= 10;
            countHit = 2;
        }

        // untuk setiap health berkurang maka speed nembak juga berkurang
        // speed = (health / static_cast<float>(maxHealth)) * maxSpeed;

        // untuk setiap health berkurang maka speed nembak bertambah, jadi semakin cepat
        speed = minSpeed + (maxSpeed - minSpeed) * ((maxHealth - health) / static_cast<float>(maxHealth - minHealth));
        damage = minDamage + (maxDamage - minDamage) * ((maxHealth - health) / static_cast<float>(maxHealth - minHealth));

        Animate(dt);
        DrawTextureV(img[currentFrame], (Vector2){centerX - img[currentFrame].width / 2, centerY - img[currentFrame].height / 2}, hit ? RED : WHITE);
    }
};

struct RingRect
{
    Rectangle rect;
    float angle;
    bool active;
};

void drawCenterText(const char *txt, int fontSize, int posY, Color color)
{
    DrawText(txt, centerX - MeasureText(txt, fontSize) / 2, posY, fontSize, color);
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Monster Orbit");
    InitAudioDevice();
    SetTargetFPS(60);

    // load audio
    Sound pewSfx = LoadSound("snd/pew.wav");

    // load image
    Texture2D playerImg = LoadTexture("img/ship_G.png");
    Texture2D rectImg = LoadTexture("img/rect_img.png");
    Texture2D bgImg = LoadTexture("img/bg.png");
    bgImg.width *= 1.5;
    bgImg.height *= 1.5;
    // ring image
    Texture2D ring1 = LoadTexture("img/Ring1.png");
    ring1.width *= 1.5;
    ring1.height *= 1.5;
    Texture2D ring2 = LoadTexture("img/Ring2.png");
    ring2.width *= 1.5;
    ring2.height *= 1.5;
    Texture2D ring3 = LoadTexture("img/Ring3.png");
    ring3.width *= 1.5;
    ring3.height *= 1.5;
    // enemy image
    Texture2D enem1_img = LoadTexture("img/enem_1.png");
    enem1_img.width *= 0.2;
    enem1_img.height *= 0.2;
    Texture2D enem2_img = LoadTexture("img/enem_2.png");
    enem2_img.width *= 0.2;
    enem2_img.height *= 0.2;
    Texture2D enem3_img = LoadTexture("img/enem_3.png");
    enem3_img.width *= 0.2;
    enem3_img.height *= 0.2;

    // player
    Player player = Player(0, 0, playerImg);
    player.radius = 200;
    player.angle = 0; // angle player saat menembak
    int speed = 5;    // speed untuk kecepatan peluru saat menembak
    float angle = 0;  // angle untuk player melihat ke tengah lingkaran
    float angle2 = 0;

    // enemy
    Enemy musuh = Enemy(WIDTH / 2, HEIGHT / 2);
    musuh.img.push_back(enem1_img);
    musuh.img.push_back(enem2_img);
    musuh.img.push_back(enem3_img);

    // other
    Vector2 ringPos = Vector2(WIDTH / 2, HEIGHT / 2);
    std::vector<RingRect> ringRects;
    int ringRectsSize = 15;
    for (int i = 0; i < ringRectsSize; i++)
    {
        RingRect rr;
        rr.rect.width = 20;
        rr.rect.height = 20;
        if (i == 8 || i == 9)
            rr.active = false;
        else
            rr.active = true;
        ringRects.push_back(rr);
    }

    std::vector<RingRect> ringRects2;
    int ringRects2Size = 15;
    for (int i = 0; i < ringRects2Size; i++)
    {
        RingRect rr;
        rr.rect.width = 20;
        rr.rect.height = 20;
        if (i == 7 || i == 6 || i == 5 || i == 4 || i == 8 || i == 9 || i == 10 || i == 11)
            rr.active = false;
        else
            rr.active = true;
        ringRects2.push_back(rr);
    }

    float ringAngle = 0;
    float ringAngle2 = 0;
    float holeAngle = 0;
    float holeAngle2 = 0;
    float dt;
    bool startGame = false;
    bool gameOver = false;
    bool musuhDead = false;
    int musuhLive = 2;
    int countdown = 5;
    float lastCount = GetTime();

    while (!WindowShouldClose())
    {
        ClearBackground(BLACK);

        dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE))
            startGame = true;

        // player shoot bullet
        if (IsKeyPressed(KEY_SPACE) && startGame && !gameOver && !musuhDead)
        {
            // kalkulasi tengah layar dengan posisi player
            Vector2 dir = (Vector2(WIDTH / 2, HEIGHT / 2) - player.pos).Normalize();
            Vector2 vel = dir * speed;
            Bullet b = Bullet(player.pos.x + player.img.width / 2, player.pos.y + player.img.height / 2, 4);
            b.vel = vel;
            PlaySound(pewSfx);
            player.bullets.push_back(b);
        }

        player.Update(startGame, gameOver, musuhDead);

        // check collision player bullets dengan musuh
        for (int i = 0; i < player.bullets.size(); i++)
        {
            if (CheckCollisionCircles(musuh.pos, 40, player.bullets[i].pos, player.bullets[i].radius))
            {
                // std::cout << "kena" << std::endl;
                musuh.hit = true;
                player.bullets.erase(player.bullets.begin() + i, player.bullets.begin() + i + 1);
            }
        }

        // check collision player bullets dengan ring 1
        for (int i = 0; i < player.bullets.size(); i++)
        {
            bool collisionOccurred = false;
            for (int j = 0; j < ringRects.size(); j++)
            {
                if (CheckCollisionCircleRec(player.bullets[i].pos, player.bullets[i].radius, ringRects[j].rect) && ringRects[j].active)
                {
                    collisionOccurred = true;
                    break;
                }
            }
            if (collisionOccurred)
            {
                player.bullets.erase(player.bullets.begin() + i, player.bullets.begin() + i + 1);
                // std::cout << "kena ring" << std::endl;
                i--; // Decrement the counter to ensure correct indexing after erasing an element
            }
        }

        // check collision player bullets dengan ring 2
        for (int i = 0; i < player.bullets.size(); i++)
        {
            bool collisionOccurred = false;
            for (int j = 0; j < ringRects2.size(); j++)
            {
                if (CheckCollisionCircleRec(player.bullets[i].pos, player.bullets[i].radius, ringRects2[j].rect) && ringRects2[j].active && musuhLive == 1)
                {
                    collisionOccurred = true;
                    break;
                }
            }
            if (collisionOccurred)
            {
                player.bullets.erase(player.bullets.begin() + i, player.bullets.begin() + i + 1);
                // std::cout << "kena ring" << std::endl;
                i--; // Decrement the counter to ensure correct indexing after erasing an element
            }
        }

        // check collision musuh bullets denga player
        for (int i = 0; i < musuh.bullets.size();)
        {
            if (CheckCollisionCircleRec(musuh.bullets[i].pos, musuh.bullets[i].radius, player.rect))
            {
                // std::cout << "kena player" << std::endl;
                if (player.health > 0)
                    player.health -= musuh.damage;
                musuh.bullets.erase(musuh.bullets.begin() + i);
            }
            else
            {
                ++i;
            }
        }

        // musuh shoot player
        if (GetTime() - musuh.lastShoot > 0.6 && startGame && !gameOver && !musuhDead)
        {
            musuh.countShot -= 1;
            musuh.lastShoot = GetTime();
        }

        if (musuh.countShot == 0)
        {
            Vector2 dirToPlayer = (player.pos - Vector2(WIDTH / 2, HEIGHT / 2)).Normalize();
            Vector2 vel = dirToPlayer * musuh.speed;
            Bullet b = Bullet(WIDTH / 2, HEIGHT / 2, 10);
            b.vel = vel;
            musuh.bullets.push_back(b);
            musuh.countShot = 2;
        }

        if (player.health <= 0)
            gameOver = true;

        if (musuh.health <= 0)
            musuhDead = true;

        if (GetTime() - lastCount > 1.0 && musuhDead && musuhLive > 1)
        {
            countdown -= 1;
            lastCount = GetTime();
        }

        if (countdown == 0 && musuhDead && musuhLive > 1)
        {
            musuhLive -= 1;
            musuhDead = false;
            musuh.health = 100;
        }

        if (musuhDead && musuhLive == 1)
            gameOver = true;

        BeginDrawing();
        // draw background
        DrawTexture(bgImg, 0, 0, WHITE);

        if (!startGame)
        {
            drawCenterText("Monster Orbit", 40, 25, WHITE);
            drawCenterText("created by aji mustofa @pepega90", 20, 75, YELLOW);
            drawCenterText("Tekan \"SPACE\" untuk play!", 40, HEIGHT - 80, WHITE);
        }

        if (musuhDead && musuhLive > 1)
        {
            drawCenterText(std::to_string(countdown).c_str(), 80, 25, WHITE);
            drawCenterText("\"Bersiaplah, pembela angkasa yang berani!\"", 27, HEIGHT - 80, RED);
        }

        if (gameOver)
        {
            drawCenterText("GAME OVER", 40, 25, RED);
            drawCenterText("Tekan \"R\" untuk restart!", 40, HEIGHT - 80, WHITE);

            if (IsKeyPressed(KEY_R))
            {
                gameOver = false;
                musuhLive = 2;
                countdown = 5;
                musuhDead = false;
                player.health = 100;
                musuh.health = 100;
            }
        }

        // draw bullet player
        for (int i = 0; i < player.bullets.size(); i++)
        {
            player.bullets[i].pos += player.bullets[i].vel;
            DrawCircleV(player.bullets[i].pos, player.bullets[i].radius, PURPLE);
        }

        // draw peluru musuh
        for (int i = 0; i < musuh.bullets.size(); /* gak lakuin increment i disini, untuk menghindari out of bonds dari iterator */)
        {
            musuh.bullets[i].pos += musuh.bullets[i].vel;
            DrawCircleV(musuh.bullets[i].pos, musuh.bullets[i].radius, RED);
            if (musuh.bullets[i].pos.x > WIDTH || musuh.bullets[i].pos.x < 0 || musuh.bullets[i].pos.y > HEIGHT || musuh.bullets[i].pos.y < 0)
            {
                musuh.bullets.erase(musuh.bullets.begin() + i);
            }
            else
            {
                // increment i jika tidak ada bullet yang di hapus
                ++i;
            }
        }

        // draw ring
        if (!gameOver)
        {
            ringAngle += 1;
            ringAngle2 -= 1;
            holeAngle += 0.5;
            holeAngle2 -= 0.5;
        }

        if (!musuhDead)
        {
            DrawTexturePro(ring1, (Rectangle){0, 0, ring1.width, ring1.height}, (Rectangle){ringPos.x, ringPos.y, ring1.width, ring1.height}, Vector2(ring1.width / 2, ring1.height / 2), ringAngle, WHITE);
        }

        if (musuhLive == 1 && !musuhDead)
        {
            DrawTexturePro(ring2, (Rectangle){0, 0, ring2.width, ring2.height}, (Rectangle){ringPos.x, ringPos.y, ring2.width, ring2.height}, Vector2(ring2.width / 2 - 50, ring2.height / 2), ringAngle2, WHITE);
        }

        // draw rotate ring rects 1
        for (int i = 0; i < ringRects.size(); i++)
        {
            // ringRects[i].angle = ringAngle * 2 * PI / 180 + i *(2 * PI/ringRectsSize) * 0.5;
            ringRects[i].angle = holeAngle * 2 * PI / 180 + i * (2 * PI / ringRectsSize);
            float radius = 65;
            float posx = ringPos.x + radius * cos(ringRects[i].angle);
            float posy = ringPos.y + radius * sin(ringRects[i].angle);
            ringRects[i].rect.x = posx - ringRects[i].rect.width / 2;
            ringRects[i].rect.y = posy - ringRects[i].rect.height / 2;

            // if(ringRects[i].active)
            //     DrawRectangleRec(ringRects[i].rect, RED);
        }

        if (musuhLive == 1)
        {
            // draw rotate ring rects 2
            for (int i = 0; i < ringRects2.size(); i++)
            {
                // ringRects2[i].angle = ringAngle * 2 * PI / 180 + i *(2 * PI/ringRects2Size) * 0.5;
                ringRects2[i].angle = holeAngle2 * 2 * PI / 180 + i * (2 * PI / ringRects2Size);
                float radius = 85;
                float posx = ringPos.x + radius * cos(ringRects2[i].angle);
                float posy = ringPos.y + radius * sin(ringRects2[i].angle);
                ringRects2[i].rect.x = posx - ringRects2[i].rect.width / 2;
                ringRects2[i].rect.y = posy - ringRects2[i].rect.height / 2;

                // if(ringRects2[i].active)
                //     DrawRectangleRec(ringRects2[i].rect, RED);
            }
        }

        // buat angle ke tengah layar dan convert ke derajat dari radian tambah 90 derajat
        angle = std::atan2((HEIGHT / 2) - player.pos.y, (WIDTH / 2) - player.pos.x) * RAD2DEG + 90;

        // draw player
        DrawTexturePro(player.img, (Rectangle){0, 0, player.img.width, player.img.height}, (Rectangle){player.pos.x + player.img.width / 2, player.pos.y + player.img.height / 2, player.img.width, player.img.height}, Vector2{(float)player.img.width / 2, (float)player.img.height / 2}, angle, WHITE);

        // draw musuh
        musuh.Update(dt);

        if (startGame && !gameOver)
        {
            // draw UI Health musuh dan player

            // draw texture UI Health
            DrawTextureEx(enem1_img, Vector2(5, 10), 0.0, 0.3, WHITE);
            DrawTextureEx(playerImg, Vector2(27, 50), 90, 0.3, WHITE);
            // draw white rectangle lines for health
            DrawRectangleLinesEx((Rectangle){38, 18, 104, 15}, 1.0, WHITE);
            DrawRectangleLinesEx((Rectangle){38, 53, 104, 15}, 1.0, WHITE);
            // draw health
            DrawRectangle(40, 20, musuh.health, 10, GREEN);
            DrawRectangle(40, 55, player.health, 10, GREEN);
        }

        // draw mouse position untuk debug

        // std::string mouseTxt = "mouse x = " + std::to_string(GetMousePosition().x) + "\n" + "mouse y = " + std::to_string(GetMousePosition().y);
        // DrawText(mouseTxt.c_str(), GetMousePosition().x + 20, GetMousePosition().y, 15, WHITE);

        EndDrawing();
    }
    UnloadTexture(enem2_img);
    UnloadTexture(enem3_img);
    UnloadTexture(enem1_img);
    UnloadTexture(playerImg);
    UnloadTexture(bgImg);
    UnloadTexture(ring1);
    UnloadTexture(ring2);
    UnloadTexture(ring3);
    UnloadSound(pewSfx);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}