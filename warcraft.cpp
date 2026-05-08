#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
using namespace std;

enum WarriorType { DRAGON, NINJA, ICEMAN, LION, WOLF };
enum WeaponType { SWORD, BOMB, ARROW };
enum Color { RED, BLUE };

const int RED_ORDER[5] = {ICEMAN, LION, WOLF, NINJA, DRAGON};
const int BLUE_ORDER[5] = {LION, DRAGON, NINJA, ICEMAN, WOLF};
const char* WARRIOR_NAMES[5] = {"dragon", "ninja", "iceman", "lion", "wolf"};
const char* WEAPON_NAMES[3] = {"sword", "bomb", "arrow"};
const char* COLOR_NAMES[2] = {"red", "blue"};

int M, N, R, K, T;
int warrior_life[5], warrior_force[5];

struct Weapon {
    int type;
    int attack;
    int use_count;

    Weapon(int t, int atk = 0) : type(t), attack(atk), use_count(0) {
        if (t == ARROW) use_count = 3;
    }

    bool is_valid() const {
        if (type == SWORD) return attack > 0;
        if (type == ARROW) return use_count > 0;
        return true;
    }
};

class Warrior {
public:
    int id;
    int life;
    int force;
    int position;
    Color color;
    WarriorType type;
    vector<Weapon> weapons;

    Warrior(int id, int life, int force, Color color, WarriorType type)
        : id(id), life(life), force(force), position(color == RED ? 0 : N + 1),
          color(color), type(type) {}

    virtual ~Warrior() {}

    virtual void born_output() const {
        printf("%s %s %d born\n", COLOR_NAMES[color], WARRIOR_NAMES[type], id);
    }

    bool has_weapon(int type) const {
        for (const auto& w : weapons)
            if (w.type == type && w.is_valid()) return true;
        return false;
    }

    Weapon* get_weapon(int type) {
        for (auto& w : weapons)
            if (w.type == type && w.is_valid()) return &w;
        return nullptr;
    }

    void remove_invalid_weapons() {
        weapons.erase(
            remove_if(weapons.begin(), weapons.end(),
                      [](const Weapon& w) { return !w.is_valid(); }),
            weapons.end());
    }

    int get_sword_attack() const {
        for (const auto& w : weapons)
            if (w.type == SWORD && w.is_valid()) return w.attack;
        return 0;
    }

    bool is_enemy_reached() const {
        if (color == RED) return position == N + 1;
        return position == 0;
    }
};

class Dragon : public Warrior {
public:
    double morale;

    Dragon(int id, int life, int force, Color color, double morale)
        : Warrior(id, life, force, color, DRAGON), morale(morale) {
        weapons.push_back(Weapon(SWORD, force * 20 / 100));
    }

    void born_output() const override {
        printf("%s %s %d born\n", COLOR_NAMES[color], WARRIOR_NAMES[type], id);
        printf("Its morale is %.2f\n", morale);
    }

    void yell(int city_num) const {
        printf("%03d:%02d %s dragon %d yelled in city %d\n",
               0, 40, COLOR_NAMES[color], id, city_num);
    }
};

class Ninja : public Warrior {
public:
    Ninja(int id, int life, int force, Color color)
        : Warrior(id, life, force, color, NINJA) {
        weapons.push_back(Weapon(SWORD, force * 20 / 100));
        weapons.push_back(Weapon(BOMB));
    }
};

class Iceman : public Warrior {
public:
    int steps;

    Iceman(int id, int life, int force, Color color)
        : Warrior(id, life, force, color, ICEMAN), steps(0) {
        weapons.push_back(Weapon(SWORD, force * 20 / 100));
    }

    void move_step() {
        steps++;
        if (steps % 2 == 0) {
            life -= 9;
            if (life <= 0) life = 1;
            force += 20;
        }
    }
};

class Lion : public Warrior {
public:
    int loyalty;
    int life_before_battle;

    Lion(int id, int life, int force, Color color, int loyalty)
        : Warrior(id, life, force, color, LION), loyalty(loyalty), life_before_battle(life) {}

    void born_output() const override {
        printf("%s %s %d born\n", COLOR_NAMES[color], WARRIOR_NAMES[type], id);
        printf("Its loyalty is %d\n", loyalty);
    }

    bool should_escape() const {
        return loyalty <= 0;
    }
};

class Wolf : public Warrior {
public:
    Wolf(int id, int life, int force, Color color)
        : Warrior(id, life, force, color, WOLF) {}
};

struct City {
    int elements;
    int flag;  // -1: no flag, 0: red, 1: blue
    int consecutive_wins;  // positive: red wins, negative: blue wins
    Warrior* warriors[2];  // [0]: red, [1]: blue

    City() : elements(0), flag(-1), consecutive_wins(0) {
        warriors[0] = nullptr;
        warriors[1] = nullptr;
    }
};

struct Headquarter {
    int elements;
    int enemy_count;
    Color color;
    int warrior_count;
    int current_index;

    Headquarter(int elements, Color color)
        : elements(elements), enemy_count(0), color(color),
          warrior_count(0), current_index(0) {}

    int get_next_type() const {
        if (color == RED) return RED_ORDER[current_index % 5];
        return BLUE_ORDER[current_index % 5];
    }
};

City cities[22];
Headquarter* red_hq;
Headquarter* blue_hq;
vector<Warrior*> all_warriors;
int current_hour;
bool war_ended;

void init() {
    for (int i = 0; i <= N + 1; i++) {
        cities[i] = City();
    }
    all_warriors.clear();
    war_ended = false;
    current_hour = 0;
}

Warrior* create_warrior(int type, int id, Color color) {
    int life = warrior_life[type];
    int force = warrior_force[type];

    switch (type) {
        case DRAGON: {
            double morale = (double)(color == RED ? red_hq->elements : blue_hq->elements) / life;
            return new Dragon(id, life, force, color, morale);
        }
        case NINJA:
            return new Ninja(id, life, force, color);
        case ICEMAN:
            return new Iceman(id, life, force, color);
        case LION: {
            int loyalty = (color == RED ? red_hq->elements : blue_hq->elements);
            return new Lion(id, life, force, color, loyalty);
        }
        case WOLF:
            return new Wolf(id, life, force, color);
    }
    return nullptr;
}

void print_time() {
    printf("%03d:%02d ", current_hour, 0);
}

bool is_valid_time() {
    int total = current_hour * 60;
    return total <= T;
}

void event_born() {
    if (!is_valid_time()) return;

    // Red
    int red_type = red_hq->get_next_type();
    if (red_hq->elements >= warrior_life[red_type]) {
        red_hq->elements -= warrior_life[red_type];
        red_hq->warrior_count++;
        Warrior* w = create_warrior(red_type, red_hq->warrior_count, RED);
        all_warriors.push_back(w);

        printf("%03d:%02d ", current_hour, 0);
        w->born_output();

        red_hq->current_index++;
    }

    // Blue
    int blue_type = blue_hq->get_next_type();
    if (blue_hq->elements >= warrior_life[blue_type]) {
        blue_hq->elements -= warrior_life[blue_type];
        blue_hq->warrior_count++;
        Warrior* w = create_warrior(blue_type, blue_hq->warrior_count, BLUE);
        all_warriors.push_back(w);

        printf("%03d:%02d ", current_hour, 0);
        w->born_output();

        blue_hq->current_index++;
    }
}

void event_lion_escape() {
    if (!is_valid_time()) return;

    for (auto w : all_warriors) {
        if (w->type == LION && !w->is_enemy_reached()) {
            Lion* lion = dynamic_cast<Lion*>(w);
            if (lion && lion->should_escape()) {
                int pos = w->position;
                int ci = w->color == RED ? 0 : 1;
                if (cities[pos].warriors[ci] == w) {
                    printf("%03d:%02d %s lion %d ran away\n",
                           current_hour, 5, COLOR_NAMES[w->color], w->id);
                    cities[pos].warriors[ci] = nullptr;
                }
            }
        }
    }
}

void event_march() {
    if (!is_valid_time()) return;

    // Clear city warriors
    for (int i = 0; i <= N + 1; i++) {
        cities[i].warriors[0] = nullptr;
        cities[i].warriors[1] = nullptr;
    }

    // Move warriors
    for (auto w : all_warriors) {
        if (w->is_enemy_reached()) continue;

        int ci = w->color == RED ? 0 : 1;

        if (w->color == RED) {
            w->position++;
        } else {
            w->position--;
        }

        // Iceman special ability - after moving
        if (w->type == ICEMAN) {
            Iceman* iceman = dynamic_cast<Iceman*>(w);
            if (iceman) iceman->move_step();
        }

        cities[w->position].warriors[ci] = w;
    }

    // Output march events
    for (int i = 0; i <= N + 1; i++) {
        for (int ci = 0; ci < 2; ci++) {
            Warrior* w = cities[i].warriors[ci];
            if (!w) continue;

            if (w->is_enemy_reached()) {
                printf("%03d:%02d %s %s %d reached %s headquarter with %d elements and force %d\n",
                       current_hour, 10, COLOR_NAMES[w->color], WARRIOR_NAMES[w->type],
                       w->id, COLOR_NAMES[1 - w->color], w->life, w->force);

                // Check if HQ taken
                Headquarter* enemy_hq = (w->color == RED) ? blue_hq : red_hq;
                enemy_hq->enemy_count++;
                if (enemy_hq->enemy_count >= 2) {
                    printf("%03d:%02d %s headquarter was taken\n",
                           current_hour, 10, COLOR_NAMES[1 - w->color]);
                    war_ended = true;
                }
            } else if (i > 0 && i <= N) {
                printf("%03d:%02d %s %s %d marched to city %d with %d elements and force %d\n",
                       current_hour, 10, COLOR_NAMES[w->color], WARRIOR_NAMES[w->type],
                       w->id, i, w->life, w->force);
            }
        }
    }
}

void event_city_produce() {
    if (!is_valid_time()) return;

    for (int i = 1; i <= N; i++) {
        cities[i].elements += 10;
    }
}

void event_collect() {
    if (!is_valid_time()) return;

    for (int i = 1; i <= N; i++) {
        if (cities[i].warriors[0] && !cities[i].warriors[1]) {
            // Only red warrior
            printf("%03d:%02d %s %s %d earned %d elements for his headquarter\n",
                   current_hour, 30, COLOR_NAMES[RED], WARRIOR_NAMES[cities[i].warriors[0]->type],
                   cities[i].warriors[0]->id, cities[i].elements);
            red_hq->elements += cities[i].elements;
            cities[i].elements = 0;
        } else if (!cities[i].warriors[0] && cities[i].warriors[1]) {
            // Only blue warrior
            printf("%03d:%02d %s %s %d earned %d elements for his headquarter\n",
                   current_hour, 30, COLOR_NAMES[BLUE], WARRIOR_NAMES[cities[i].warriors[1]->type],
                   cities[i].warriors[1]->id, cities[i].elements);
            blue_hq->elements += cities[i].elements;
            cities[i].elements = 0;
        }
    }
}

void event_arrow() {
    if (!is_valid_time()) return;

    for (int i = 1; i <= N; i++) {
        // Red warrior shoots east
        if (cities[i].warriors[0] && cities[i].warriors[0]->has_weapon(ARROW)) {
            if (i + 1 <= N && cities[i + 1].warriors[1]) {
                Weapon* arrow = cities[i].warriors[0]->get_weapon(ARROW);
                if (arrow) {
                    cities[i + 1].warriors[1]->life -= R;
                    arrow->use_count--;

                    if (cities[i + 1].warriors[1]->life <= 0) {
                        printf("%03d:%02d %s %s %d shot and killed %s %s %d\n",
                               current_hour, 35, COLOR_NAMES[RED],
                               WARRIOR_NAMES[cities[i].warriors[0]->type],
                               cities[i].warriors[0]->id,
                               COLOR_NAMES[BLUE],
                               WARRIOR_NAMES[cities[i + 1].warriors[1]->type],
                               cities[i + 1].warriors[1]->id);
                    } else {
                        printf("%03d:%02d %s %s %d shot\n",
                               current_hour, 35, COLOR_NAMES[RED],
                               WARRIOR_NAMES[cities[i].warriors[0]->type],
                               cities[i].warriors[0]->id);
                    }
                }
            }
        }

        // Blue warrior shoots west
        if (cities[i].warriors[1] && cities[i].warriors[1]->has_weapon(ARROW)) {
            if (i - 1 >= 1 && cities[i - 1].warriors[0]) {
                Weapon* arrow = cities[i].warriors[1]->get_weapon(ARROW);
                if (arrow) {
                    cities[i - 1].warriors[0]->life -= R;
                    arrow->use_count--;

                    if (cities[i - 1].warriors[0]->life <= 0) {
                        printf("%03d:%02d %s %s %d shot and killed %s %s %d\n",
                               current_hour, 35, COLOR_NAMES[BLUE],
                               WARRIOR_NAMES[cities[i].warriors[1]->type],
                               cities[i].warriors[1]->id,
                               COLOR_NAMES[RED],
                               WARRIOR_NAMES[cities[i - 1].warriors[0]->type],
                               cities[i - 1].warriors[0]->id);
                    } else {
                        printf("%03d:%02d %s %s %d shot\n",
                               current_hour, 35, COLOR_NAMES[BLUE],
                               WARRIOR_NAMES[cities[i].warriors[1]->type],
                               cities[i].warriors[1]->id);
                    }
                }
            }
        }
    }
}

void event_bomb() {
    if (!is_valid_time()) return;

    for (int i = 1; i <= N; i++) {
        Warrior* red = cities[i].warriors[0];
        Warrior* blue = cities[i].warriors[1];

        if (!red || !blue) continue;
        if (red->life <= 0 || blue->life <= 0) continue;

        // Determine who attacks first
        bool red_first;
        if (cities[i].flag == 0) red_first = true;
        else if (cities[i].flag == 1) red_first = false;
        else red_first = (i % 2 == 1);

        Warrior* attacker = red_first ? red : blue;
        Warrior* defender = red_first ? blue : red;

        // Check if bomb should be used
        if (red->has_weapon(BOMB) || blue->has_weapon(BOMB)) {
            int attacker_damage = attacker->force + attacker->get_sword_attack();
            int defender_life_after = defender->life - attacker_damage;

            bool defender_dies = defender_life_after <= 0;
            bool attacker_dies = false;

            if (!defender_dies && defender->type != NINJA) {
                int counter_damage = defender->force / 2 + defender->get_sword_attack();
                int attacker_life_after = attacker->life - counter_damage;
                attacker_dies = attacker_life_after <= 0;
            }

            if (red->has_weapon(BOMB) && ((red_first && attacker_dies) || (!red_first && defender_dies))) {
                printf("%03d:%02d %s %s %d used a bomb and killed %s %s %d\n",
                       current_hour, 38, COLOR_NAMES[RED], WARRIOR_NAMES[red->type], red->id,
                       COLOR_NAMES[BLUE], WARRIOR_NAMES[blue->type], blue->id);
                red->life = 0;
                blue->life = 0;
                cities[i].warriors[0] = nullptr;
                cities[i].warriors[1] = nullptr;
                continue;
            }

            if (blue->has_weapon(BOMB) && ((!red_first && attacker_dies) || (red_first && defender_dies))) {
                printf("%03d:%02d %s %s %d used a bomb and killed %s %s %d\n",
                       current_hour, 38, COLOR_NAMES[BLUE], WARRIOR_NAMES[blue->type], blue->id,
                       COLOR_NAMES[RED], WARRIOR_NAMES[red->type], red->id);
                red->life = 0;
                blue->life = 0;
                cities[i].warriors[0] = nullptr;
                cities[i].warriors[1] = nullptr;
                continue;
            }
        }
    }
}

void event_battle() {
    if (!is_valid_time()) return;

    vector<pair<int, int>> rewards;  // (city, color)

    for (int i = 1; i <= N; i++) {
        Warrior* red = cities[i].warriors[0];
        Warrior* blue = cities[i].warriors[1];

        if (!red || !blue) continue;

        // Check if already dead from arrow
        if (red->life <= 0 && blue->life <= 0) {
            cities[i].warriors[0] = nullptr;
            cities[i].warriors[1] = nullptr;
            continue;
        }

        if (red->life <= 0) {
            // Blue wins, red killed by arrow
            // No "was killed" output for arrow deaths

            // Wolf captures weapons
            if (blue->type == WOLF) {
                for (auto& w : red->weapons) {
                    if (w.is_valid() && !blue->has_weapon(w.type)) {
                        blue->weapons.push_back(w);
                    }
                }
            }

            // Dragon morale
            if (blue->type == DRAGON) {
                Dragon* dragon = dynamic_cast<Dragon*>(blue);
                if (dragon) {
                    dragon->morale += 0.2;
                    if (dragon->morale > 0.8) {
                        dragon->yell(i);
                    }
                }
            }

            // Lion captures life
            if (blue->type == LION) {
                Lion* lion = dynamic_cast<Lion*>(blue);
                if (lion) {
                    blue->life += lion->life_before_battle;
                }
            }

            rewards.push_back({i, BLUE});

            // Update consecutive wins
            if (cities[i].consecutive_wins < 0) {
                cities[i].consecutive_wins--;
            } else {
                cities[i].consecutive_wins = -1;
            }

            if (cities[i].consecutive_wins == -2) {
                if (cities[i].flag != 1) {
                    printf("%03d:%02d blue flag raised in city %d\n", current_hour, 40, i);
                    cities[i].flag = 1;
                }
            }

            cities[i].warriors[0] = nullptr;
            continue;
        }

        if (blue->life <= 0) {
            // Red wins, blue killed by arrow
            // No "was killed" output for arrow deaths

            // Wolf captures weapons
            if (red->type == WOLF) {
                for (auto& w : blue->weapons) {
                    if (w.is_valid() && !red->has_weapon(w.type)) {
                        red->weapons.push_back(w);
                    }
                }
            }

            // Dragon morale
            if (red->type == DRAGON) {
                Dragon* dragon = dynamic_cast<Dragon*>(red);
                if (dragon) {
                    dragon->morale += 0.2;
                    if (dragon->morale > 0.8) {
                        dragon->yell(i);
                    }
                }
            }

            // Lion captures life
            if (red->type == LION) {
                Lion* lion = dynamic_cast<Lion*>(red);
                if (lion) {
                    red->life += lion->life_before_battle;
                }
            }

            rewards.push_back({i, RED});

            // Update consecutive wins
            if (cities[i].consecutive_wins > 0) {
                cities[i].consecutive_wins++;
            } else {
                cities[i].consecutive_wins = 1;
            }

            if (cities[i].consecutive_wins == 2) {
                if (cities[i].flag != 0) {
                    printf("%03d:%02d red flag raised in city %d\n", current_hour, 40, i);
                    cities[i].flag = 0;
                }
            }

            cities[i].warriors[1] = nullptr;
            continue;
        }

        // Both alive, determine who attacks first
        bool red_first;
        if (cities[i].flag == 0) red_first = true;
        else if (cities[i].flag == 1) red_first = false;
        else red_first = (i % 2 == 1);

        Warrior* attacker = red_first ? red : blue;
        Warrior* defender = red_first ? blue : red;

        // Store life before battle for lion
        if (attacker->type == LION) {
            Lion* lion = dynamic_cast<Lion*>(attacker);
            if (lion) lion->life_before_battle = attacker->life;
        }
        if (defender->type == LION) {
            Lion* lion = dynamic_cast<Lion*>(defender);
            if (lion) lion->life_before_battle = defender->life;
        }

        // Attacker attacks
        int attack_damage = attacker->force + attacker->get_sword_attack();
        defender->life -= attack_damage;

        printf("%03d:%02d %s %s %d attacked %s %s %d in city %d with %d elements and force %d\n",
               current_hour, 40, COLOR_NAMES[attacker->color], WARRIOR_NAMES[attacker->type],
               attacker->id, COLOR_NAMES[defender->color], WARRIOR_NAMES[defender->type],
               defender->id, i, attacker->life, attacker->force);

        // Sword degrades
        for (auto& w : attacker->weapons) {
            if (w.type == SWORD && w.is_valid()) {
                w.attack = w.attack * 80 / 100;
            }
        }

        if (defender->life <= 0) {
            // Defender dies
            printf("%03d:%02d %s %s %d was killed in city %d\n",
                   current_hour, 40, COLOR_NAMES[defender->color], WARRIOR_NAMES[defender->type],
                   defender->id, i);

            // Wolf captures weapons
            if (attacker->type == WOLF) {
                for (auto& w : defender->weapons) {
                    if (w.is_valid() && !attacker->has_weapon(w.type)) {
                        attacker->weapons.push_back(w);
                    }
                }
            }

            // Dragon morale and yell
            if (attacker->type == DRAGON) {
                Dragon* dragon = dynamic_cast<Dragon*>(attacker);
                if (dragon) {
                    dragon->morale += 0.2;
                    // Dragon yells if it was the attacker and morale > 0.8
                    if ((attacker->color == RED && red_first) || (attacker->color == BLUE && !red_first)) {
                        if (dragon->morale > 0.8) {
                            dragon->yell(i);
                        }
                    }
                }
            }

            // Lion loyalty - only decrease if didn't kill enemy
            // Actually attacker killed enemy, so loyalty doesn't decrease

            rewards.push_back({i, attacker->color});

            // Update consecutive wins
            int ci = attacker->color == RED ? 1 : -1;
            if ((cities[i].consecutive_wins > 0 && ci > 0) ||
                (cities[i].consecutive_wins < 0 && ci < 0)) {
                cities[i].consecutive_wins += ci;
            } else {
                cities[i].consecutive_wins = ci;
            }

            if (cities[i].consecutive_wins == 2 || cities[i].consecutive_wins == -2) {
                int flag_color = cities[i].consecutive_wins > 0 ? 0 : 1;
                if (cities[i].flag != flag_color) {
                    printf("%03d:%02d %s flag raised in city %d\n",
                           current_hour, 40, COLOR_NAMES[flag_color], i);
                    cities[i].flag = flag_color;
                }
            }

            if (defender->color == RED) cities[i].warriors[0] = nullptr;
            else cities[i].warriors[1] = nullptr;
        } else {
            // Defender counterattacks (if not ninja)
            if (defender->type != NINJA) {
                int counter_damage = defender->force / 2 + defender->get_sword_attack();
                attacker->life -= counter_damage;

                printf("%03d:%02d %s %s %d fought back against %s %s %d in city %d\n",
                       current_hour, 40, COLOR_NAMES[defender->color], WARRIOR_NAMES[defender->type],
                       defender->id, COLOR_NAMES[attacker->color], WARRIOR_NAMES[attacker->type],
                       attacker->id, i);

                // Sword degrades
                for (auto& w : defender->weapons) {
                    if (w.type == SWORD && w.is_valid()) {
                        w.attack = w.attack * 80 / 100;
                    }
                }

                if (attacker->life <= 0) {
                    // Attacker dies
                    printf("%03d:%02d %s %s %d was killed in city %d\n",
                           current_hour, 40, COLOR_NAMES[attacker->color], WARRIOR_NAMES[attacker->type],
                           attacker->id, i);

                    // Wolf captures weapons
                    if (defender->type == WOLF) {
                        for (auto& w : attacker->weapons) {
                            if (w.is_valid() && !defender->has_weapon(w.type)) {
                                defender->weapons.push_back(w);
                            }
                        }
                    }

                    // Dragon morale - doesn't yell if died
                    if (defender->type == DRAGON) {
                        Dragon* dragon = dynamic_cast<Dragon*>(defender);
                        if (dragon) {
                            dragon->morale += 0.2;
                            // Defender yells if it was attacked (not the attacker)
                            if ((defender->color == RED && !red_first) || (defender->color == BLUE && red_first)) {
                                if (dragon->morale > 0.8) {
                                    dragon->yell(i);
                                }
                            }
                        }
                    }

                    // Lion loyalty - defender killed enemy, so loyalty doesn't decrease

                    rewards.push_back({i, defender->color});

                    // Update consecutive wins
                    int ci = defender->color == RED ? 1 : -1;
                    if ((cities[i].consecutive_wins > 0 && ci > 0) ||
                        (cities[i].consecutive_wins < 0 && ci < 0)) {
                        cities[i].consecutive_wins += ci;
                    } else {
                        cities[i].consecutive_wins = ci;
                    }

                    if (cities[i].consecutive_wins == 2 || cities[i].consecutive_wins == -2) {
                        int flag_color = cities[i].consecutive_wins > 0 ? 0 : 1;
                        if (cities[i].flag != flag_color) {
                            printf("%03d:%02d %s flag raised in city %d\n",
                                   current_hour, 40, COLOR_NAMES[flag_color], i);
                            cities[i].flag = flag_color;
                        }
                    }

                    if (attacker->color == RED) cities[i].warriors[0] = nullptr;
                    else cities[i].warriors[1] = nullptr;
                } else {
                    // Both survive - draw
                    cities[i].consecutive_wins = 0;

                    // Dragon morale decreases
                    if (attacker->type == DRAGON) {
                        Dragon* dragon = dynamic_cast<Dragon*>(attacker);
                        if (dragon) dragon->morale -= 0.2;
                    }
                    if (defender->type == DRAGON) {
                        Dragon* dragon = dynamic_cast<Dragon*>(defender);
                        if (dragon) dragon->morale -= 0.2;
                    }

                    // Lion loyalty decreases
                    if (attacker->type == LION) {
                        Lion* lion = dynamic_cast<Lion*>(attacker);
                        if (lion) lion->loyalty -= K;
                    }
                    if (defender->type == LION) {
                        Lion* lion = dynamic_cast<Lion*>(defender);
                        if (lion) lion->loyalty -= K;
                    }
                }
            } else {
                // Both survive - draw (ninja doesn't counterattack)
                cities[i].consecutive_wins = 0;

                // Dragon morale decreases
                if (attacker->type == DRAGON) {
                    Dragon* dragon = dynamic_cast<Dragon*>(attacker);
                    if (dragon) dragon->morale -= 0.2;
                }
                if (defender->type == DRAGON) {
                    Dragon* dragon = dynamic_cast<Dragon*>(defender);
                    if (dragon) dragon->morale -= 0.2;
                }

                // Lion loyalty decreases
                if (attacker->type == LION) {
                    Lion* lion = dynamic_cast<Lion*>(attacker);
                    if (lion) lion->loyalty -= K;
                }
                if (defender->type == LION) {
                    Lion* lion = dynamic_cast<Lion*>(defender);
                    if (lion) lion->loyalty -= K;
                }
            }
        }
    }

    // Reward winners
    // Sort by distance to enemy HQ (closer first)
    // For red: larger city number = closer to blue HQ
    // For blue: smaller city number = closer to red HQ
    sort(rewards.begin(), rewards.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        if (a.second != b.second) {
            return a.second == RED;  // Red before blue
        }
        if (a.second == RED) {
            return a.first > b.first;  // Larger city number first for red
        }
        return a.first < b.first;  // Smaller city number first for blue
    });

    for (auto& r : rewards) {
        int city = r.first;
        Color color = (Color)r.second;
        Headquarter* hq = (color == RED) ? red_hq : blue_hq;

        if (hq->elements >= 8) {
            hq->elements -= 8;
            Warrior* w = (color == RED) ? cities[city].warriors[0] : cities[city].warriors[1];
            if (w) {
                w->life += 8;
                printf("%03d:%02d %s %s %d earned %d elements for his headquarter\n",
                       current_hour, 40, COLOR_NAMES[color], WARRIOR_NAMES[w->type],
                       w->id, 8);
            }
        }
    }

    // Collect city elements
    for (int i = 1; i <= N; i++) {
        if (cities[i].warriors[0] && !cities[i].warriors[1]) {
            red_hq->elements += cities[i].elements;
            cities[i].elements = 0;
        } else if (!cities[i].warriors[0] && cities[i].warriors[1]) {
            blue_hq->elements += cities[i].elements;
            cities[i].elements = 0;
        }
    }
}

void event_report_elements() {
    if (!is_valid_time()) return;

    printf("%03d:%02d %d elements in red headquarter\n", current_hour, 50, red_hq->elements);
    printf("%03d:%02d %d elements in blue headquarter\n", current_hour, 50, blue_hq->elements);
}

void event_report_weapons() {
    if (!is_valid_time()) return;

    for (int i = 0; i <= N + 1; i++) {
        // Red warriors
        if (cities[i].warriors[0]) {
            Warrior* w = cities[i].warriors[0];
            w->remove_invalid_weapons();

            printf("%03d:%02d %s %s %d has", current_hour, 55, COLOR_NAMES[RED],
                   WARRIOR_NAMES[w->type], w->id);

            if (w->weapons.empty()) {
                printf(" no weapon\n");
            } else {
                // Sort: arrow, bomb, sword (ARROW=2, BOMB=1, SWORD=0)
                vector<Weapon> sorted_weapons = w->weapons;
                sort(sorted_weapons.begin(), sorted_weapons.end(),
                     [](const Weapon& a, const Weapon& b) { return a.type > b.type; });

                bool first = true;
                for (auto& wp : sorted_weapons) {
                    if (!wp.is_valid()) continue;
                    if (!first) printf(",");
                    first = false;

                    if (wp.type == ARROW) {
                        printf("arrow(%d)", wp.use_count);
                    } else if (wp.type == BOMB) {
                        printf("bomb");
                    } else if (wp.type == SWORD) {
                        printf("sword(%d)", wp.attack);
                    }
                }
                printf("\n");
            }
        }
    }

    for (int i = 0; i <= N + 1; i++) {
        // Blue warriors
        if (cities[i].warriors[1]) {
            Warrior* w = cities[i].warriors[1];
            w->remove_invalid_weapons();

            printf("%03d:%02d %s %s %d has", current_hour, 55, COLOR_NAMES[BLUE],
                   WARRIOR_NAMES[w->type], w->id);

            if (w->weapons.empty()) {
                printf(" no weapon\n");
            } else {
                // Sort: arrow, bomb, sword (ARROW=2, BOMB=1, SWORD=0)
                vector<Weapon> sorted_weapons = w->weapons;
                sort(sorted_weapons.begin(), sorted_weapons.end(),
                     [](const Weapon& a, const Weapon& b) { return a.type > b.type; });

                bool first = true;
                for (auto& wp : sorted_weapons) {
                    if (!wp.is_valid()) continue;
                    if (!first) printf(",");
                    first = false;

                    if (wp.type == ARROW) {
                        printf("arrow(%d)", wp.use_count);
                    } else if (wp.type == BOMB) {
                        printf("bomb");
                    } else if (wp.type == SWORD) {
                        printf("sword(%d)", wp.attack);
                    }
                }
                printf("\n");
            }
        }
    }
}

void clean_dead_warriors() {
    all_warriors.erase(
        remove_if(all_warriors.begin(), all_warriors.end(),
                  [](Warrior* w) {
                      if (w->life <= 0) {
                          delete w;
                          return true;
                      }
                      return false;
                  }),
        all_warriors.end());
}

int main() {
    int t;
    scanf("%d", &t);

    for (int case_num = 1; case_num <= t; case_num++) {
        scanf("%d %d %d %d %d", &M, &N, &R, &K, &T);

        for (int i = 0; i < 5; i++) scanf("%d", &warrior_life[i]);
        for (int i = 0; i < 5; i++) scanf("%d", &warrior_force[i]);

        printf("Case %d:\n", case_num);

        init();
        red_hq = new Headquarter(M, RED);
        blue_hq = new Headquarter(M, BLUE);

        for (current_hour = 0; current_hour * 60 <= T && !war_ended; current_hour++) {
            event_born();
            if (war_ended) break;

            event_lion_escape();
            if (war_ended) break;

            event_march();
            if (war_ended) break;

            event_city_produce();
            if (war_ended) break;

            event_collect();
            if (war_ended) break;

            event_arrow();
            if (war_ended) break;

            event_bomb();
            if (war_ended) break;

            event_battle();
            if (war_ended) break;

            event_report_elements();
            if (war_ended) break;

            event_report_weapons();
            if (war_ended) break;

            clean_dead_warriors();
        }

        // Clean up
        for (auto w : all_warriors) delete w;
        all_warriors.clear();
        delete red_hq;
        delete blue_hq;
    }

    return 0;
}
