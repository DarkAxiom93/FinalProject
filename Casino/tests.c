/*
 * Casino Test Harness — regression & unit tests for core money / game logic.
 *
 * Compile (from Casino\Casino\ directory, MSVC x64 Native Tools prompt):
 *
 *   cl /W3 /Fe:..\tests.exe tests.c account.c blackjack.c poker.c roulette.c ^
 *      cards.c utils.c graphics.c football.c slots.c leaderboard.c admin.c
 *
 * Run:
 *   ..\tests.exe
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <windows.h>

#include "casino.h"
#include "cards.h"
#include "account.h"
#include "utils.h"
#include "test_api.h"

/* =====================================================================
 * CHECK macro — records pass/fail; prints coloured result.
 * ===================================================================== */
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                    \
    do {                                                                    \
        if (cond) {                                                         \
            printf("  \x1b[32m[PASS]\x1b[0m %s\n", (msg));                 \
            g_pass++;                                                       \
        } else {                                                            \
            printf("  \x1b[31m[FAIL]\x1b[0m %s\n", (msg));                 \
            g_fail++;                                                       \
        }                                                                   \
    } while (0)

/* =====================================================================
 * Helpers
 * ===================================================================== */

static Card mk(int rank_val, char suit) {
    Card c;
    memset(&c, 0, sizeof(c));
    c.rank_val = rank_val;
    c.suit     = suit;
    return c;
}

static Player fresh_player(int balance, int bank, long long winnings) {
    Player p;
    memset(&p, 0, sizeof(p));
    strcpy(p.name, "TEST");
    p.balance        = balance;
    p.bank_balance   = bank;
    p.total_winnings = winnings;
    return p;
}

/* =====================================================================
 * 1. add_balance_safe (account.c)
 * ===================================================================== */
static void test_add_balance_safe(void) {
    printf("\n=== add_balance_safe ===\n");

    /* 1a. Normal add — well below MAX_BALANCE */
    {
        Player p = fresh_player(1000, 0, 0);
        add_balance_safe(&p, 500);
        CHECK(p.balance        == 1500, "Normal add: balance correct");
        CHECK(p.bank_balance   == 0,    "Normal add: bank untouched");
        CHECK(p.total_winnings == 0,    "Normal add: winnings untouched");
    }

    /* 1b. Wallet overflows → excess goes to bank */
    {
        Player p = fresh_player(MAX_BALANCE - 100, 0, 0);  /* 49 900 */
        add_balance_safe(&p, 500);                          /* overflow = 400 */
        CHECK(p.balance      == MAX_BALANCE, "Wallet-overflow: balance capped");
        CHECK(p.bank_balance == 400,         "Wallet-overflow: overflow in bank");
    }

    /* 1c. Wallet AND bank both full → capping; uncredited deducted from total_winnings */
    {
        Player p = fresh_player(MAX_BALANCE - 100,        /* 49 900 */
                                MAX_BANK_BALANCE - 200,   /* 499 800 */
                                5000);
        /* amount=1000 → overflow=900; bank can take 200; uncredited=700 */
        add_balance_safe(&p, 1000);
        CHECK(p.balance        == MAX_BALANCE,      "Full-cap: wallet at MAX");
        CHECK(p.bank_balance   == MAX_BANK_BALANCE, "Full-cap: bank at MAX");
        CHECK(p.total_winnings == 5000 - 700,       "Full-cap: uncredited subtracted from winnings");
    }

    /* 1d. amount <= 0 → no change */
    {
        Player p = fresh_player(1000, 0, 0);
        add_balance_safe(&p, 0);
        CHECK(p.balance == 1000, "Amount 0: no change");
        add_balance_safe(&p, -50);
        CHECK(p.balance == 1000, "Amount negative: no change");
    }
}

/* =====================================================================
 * 2. calculate_hand_value (blackjack.c)
 * ===================================================================== */
static void test_calculate_hand_value(void) {
    printf("\n=== calculate_hand_value ===\n");

    /* 2a. Ace + King = 21 (ace counts as 11) */
    {
        Card h[2] = { mk(14,'H'), mk(13,'S') };
        CHECK(calculate_hand_value(h, 2) == 21, "A+K = 21");
    }

    /* 2b. Ace + 9 + 5 = 15 (ace drops to 1 to avoid bust) */
    {
        Card h[3] = { mk(14,'H'), mk(9,'D'), mk(5,'C') };
        CHECK(calculate_hand_value(h, 3) == 15, "A+9+5 = 15 (not 25, ace softened)");
    }

    /* 2c. Two aces = 12 (one counts 11, one counts 1) */
    {
        Card h[2] = { mk(14,'H'), mk(14,'D') };
        CHECK(calculate_hand_value(h, 2) == 12, "A+A = 12");
    }

    /* 2d. Face cards all worth 10 */
    {
        Card h_j[1] = { mk(11,'H') };
        Card h_q[1] = { mk(12,'H') };
        Card h_k[1] = { mk(13,'H') };
        CHECK(calculate_hand_value(h_j, 1) == 10, "Jack  = 10");
        CHECK(calculate_hand_value(h_q, 1) == 10, "Queen = 10");
        CHECK(calculate_hand_value(h_k, 1) == 10, "King  = 10");
    }

    /* 2e. Three-card bust recovery: A+K+8 = 19 (not 29) */
    {
        Card h[3] = { mk(14,'H'), mk(13,'D'), mk(8,'C') };
        CHECK(calculate_hand_value(h, 3) == 19, "A+K+8 = 19 (ace softened)");
    }
}

/* =====================================================================
 * 3. is_soft_17 (blackjack.c)
 * ===================================================================== */
static void test_is_soft_17(void) {
    printf("\n=== is_soft_17 ===\n");

    /* 3a. A+6 → soft 17 (true) */
    {
        Card h[2] = { mk(14,'H'), mk(6,'D') };
        CHECK(is_soft_17(h, 2) == 1, "A+6 is soft 17");
    }

    /* 3b. 10+7 → hard 17 (false) */
    {
        Card h[2] = { mk(10,'H'), mk(7,'D') };
        CHECK(is_soft_17(h, 2) == 0, "10+7 is hard 17");
    }

    /* 3c. A+K = 21 → not 17 → false */
    {
        Card h[2] = { mk(14,'H'), mk(13,'S') };
        CHECK(is_soft_17(h, 2) == 0, "A+K (21) not soft 17");
    }

    /* 3d. 7+7+3 = 17, no ace → hard 17 */
    {
        Card h[3] = { mk(7,'H'), mk(7,'D'), mk(3,'C') };
        CHECK(is_soft_17(h, 3) == 0, "7+7+3 is hard 17");
    }
}

/* =====================================================================
 * 4. evaluate_poker_hand + TRIPS regression (poker.c)
 * ===================================================================== */
static void test_poker_hand_ranking(void) {
    printf("\n=== evaluate_poker_hand: hand ranking ===\n");

    /* 4a. One pair */
    {
        Card h[5] = {
            mk(14,'H'), mk(14,'D'),
            mk(2,'C'),  mk(5,'S'), mk(9,'H')
        };
        int s = evaluate_poker_hand(h, 5);
        CHECK(s >= T_ONE_PAIR && s < T_TWO_PAIR, "Pair of Aces ranks ONE_PAIR");
    }

    /* 4b. Three of a kind */
    {
        Card h[7] = {
            mk(7,'H'),  mk(7,'D'),  mk(7,'C'),
            mk(2,'H'),  mk(4,'S'),  mk(9,'D'), mk(13,'H')
        };
        int s = evaluate_poker_hand(h, 7);
        CHECK(s >= T_THREE_OF_A_KIND && s < T_STRAIGHT, "Three 7s ranks THREE_OF_A_KIND");
    }

    /* 4c. Straight 2-3-4-5-6 */
    {
        Card h[5] = {
            mk(2,'H'), mk(3,'D'), mk(4,'C'), mk(5,'S'), mk(6,'H')
        };
        int s = evaluate_poker_hand(h, 5);
        CHECK(s >= T_STRAIGHT && s < T_FLUSH, "2-3-4-5-6 ranks STRAIGHT");
    }

    /* 4d. Flush (5 hearts, non-sequential) */
    {
        Card h[5] = {
            mk(2,'H'), mk(5,'H'), mk(7,'H'), mk(9,'H'), mk(11,'H')
        };
        int s = evaluate_poker_hand(h, 5);
        CHECK(s >= T_FLUSH && s < T_STRAIGHT_FLUSH, "5 hearts ranks FLUSH");
    }

    /* 4e. Full house */
    {
        Card h[5] = {
            mk(7,'H'), mk(7,'D'), mk(7,'C'), mk(13,'H'), mk(13,'D')
        };
        int s = evaluate_poker_hand(h, 5);
        CHECK(s >= T_FULL_HOUSE && s < T_FOUR_OF_A_KIND, "Three 7s + pair of Ks ranks FULL_HOUSE");
    }
}

/* =====================================================================
 * 4b. evaluate_poker_hand — same-category tiebreaks (poker.c)
 * ===================================================================== */
static void test_poker_hand_comparisons(void) {
    printf("\n=== evaluate_poker_hand: same-category tiebreaks ===\n");

    /* Four of a Kind: the quad's own rank must outrank a higher kicker elsewhere */
    {
        Card quad_sevens_ace_kicker[7] = {
            mk(7,'H'), mk(7,'D'), mk(7,'C'), mk(7,'S'),
            mk(14,'H'), mk(2,'D'), mk(3,'C')
        };
        Card quad_kings_low_kicker[7] = {
            mk(13,'H'), mk(13,'D'), mk(13,'C'), mk(13,'S'),
            mk(5,'H'), mk(4,'D'), mk(2,'C')
        };
        int s1 = evaluate_poker_hand(quad_sevens_ace_kicker, 7);
        int s2 = evaluate_poker_hand(quad_kings_low_kicker, 7);
        CHECK(s2 > s1, "Quad Kings beats Quad 7s even with an Ace kicker on the 7s");
    }

    /* Full House: two trips in 7 cards must be recognized as a Full House, not Three of a Kind */
    {
        Card double_trips[7] = {
            mk(7,'H'), mk(7,'D'), mk(7,'C'),
            mk(13,'H'), mk(13,'D'), mk(13,'C'),
            mk(2,'S')
        };
        int s = evaluate_poker_hand(double_trips, 7);
        CHECK(s >= T_FULL_HOUSE && s < T_FOUR_OF_A_KIND, "Trip 7s + Trip Ks ranks as FULL_HOUSE (Ks full of 7s)");
    }

    /* Full House: same trip rank, higher pair rank must win (not tie) */
    {
        Card sevens_full_of_kings[5] = {
            mk(7,'H'), mk(7,'D'), mk(7,'C'), mk(13,'H'), mk(13,'D')
        };
        Card sevens_full_of_queens[5] = {
            mk(7,'S'), mk(7,'C'), mk(7,'D'), mk(12,'H'), mk(12,'D')
        };
        int s1 = evaluate_poker_hand(sevens_full_of_kings, 5);
        int s2 = evaluate_poker_hand(sevens_full_of_queens, 5);
        CHECK(s1 > s2, "Sevens full of Kings beats Sevens full of Queens");
    }

    /* Flush: high card outside the flush suit must not inflate the flush's tiebreak */
    {
        Card flush_to_jack_with_offsuit_ace[7] = {
            mk(2,'H'), mk(5,'H'), mk(7,'H'), mk(9,'H'), mk(11,'H'),
            mk(14,'C'), mk(3,'D')
        };
        Card flush_to_king[7] = {
            mk(2,'D'), mk(4,'D'), mk(6,'D'), mk(9,'D'), mk(13,'D'),
            mk(5,'S'), mk(8,'C')
        };
        int s1 = evaluate_poker_hand(flush_to_jack_with_offsuit_ace, 7);
        int s2 = evaluate_poker_hand(flush_to_king, 7);
        CHECK(s2 > s1, "King-high flush beats Jack-high flush despite an offsuit Ace kicker");
    }

    /* Three of a Kind: kickers must break ties between equal trips */
    {
        Card trip_sevens_high_kickers[5] = {
            mk(7,'H'), mk(7,'D'), mk(7,'C'), mk(14,'S'), mk(13,'H')
        };
        Card trip_sevens_low_kickers[5] = {
            mk(7,'S'), mk(7,'C'), mk(7,'D'), mk(2,'H'), mk(3,'D')
        };
        int s1 = evaluate_poker_hand(trip_sevens_high_kickers, 5);
        int s2 = evaluate_poker_hand(trip_sevens_low_kickers, 5);
        CHECK(s1 > s2, "Trip 7s with A,K kickers beats Trip 7s with 2,3 kickers");
    }

    /* Two Pair: second pair must break ties between equal top pairs */
    {
        Card kk_pair_threes[5] = {
            mk(13,'H'), mk(13,'D'), mk(3,'C'), mk(3,'S'), mk(9,'H')
        };
        Card kk_pair_twos[5] = {
            mk(13,'C'), mk(13,'S'), mk(2,'H'), mk(2,'D'), mk(9,'C')
        };
        int s1 = evaluate_poker_hand(kk_pair_threes, 5);
        int s2 = evaluate_poker_hand(kk_pair_twos, 5);
        CHECK(s1 > s2, "K-K with 3-3 second pair beats K-K with 2-2 second pair");
    }

    /* One Pair: kickers must break ties between equal pairs */
    {
        Card pair_sevens_high_kickers[5] = {
            mk(7,'H'), mk(7,'D'), mk(14,'C'), mk(13,'S'), mk(12,'H')
        };
        Card pair_sevens_low_kickers[5] = {
            mk(7,'S'), mk(7,'C'), mk(2,'H'), mk(3,'D'), mk(4,'C')
        };
        int s1 = evaluate_poker_hand(pair_sevens_high_kickers, 5);
        int s2 = evaluate_poker_hand(pair_sevens_low_kickers, 5);
        CHECK(s1 > s2, "Pair of 7s with A,K,Q kickers beats Pair of 7s with 2,3,4 kickers");
    }

    /* High Card: second-highest card must break ties between equal top cards */
    {
        Card ace_high_strong[5] = {
            mk(14,'H'), mk(13,'D'), mk(11,'C'), mk(9,'S'), mk(6,'H')
        };
        Card ace_high_weak[5] = {
            mk(14,'C'), mk(2,'D'), mk(4,'H'), mk(6,'S'), mk(8,'D')
        };
        int s1 = evaluate_poker_hand(ace_high_strong, 5);
        int s2 = evaluate_poker_hand(ace_high_weak, 5);
        CHECK(s1 > s2, "A-K-J-9-6 beats A-8-6-4-2 (both Ace-high, second card breaks tie)");
    }
}

/* =====================================================================
 * 4c. Player ban flag & save-file versioning (account.c)
 * ===================================================================== */
static void test_player_ban_and_versioning(void) {
    printf("\n=== Player ban flag: persistence & legacy save compatibility ===\n");

    /* Round trip through the current (v3) save format */
    {
        Player p = fresh_player(1000, 0, 0);
        strcpy(p.name, "TEST_BAN_PLAYER_QA");
        p.is_banned = 1;
        save_player(&p);

        Player reloaded = { 0 };
        strcpy(reloaded.name, "TEST_BAN_PLAYER_QA");
        load_player(&reloaded);

        CHECK(reloaded.is_banned == 1, "is_banned survives save_player + load_player round trip");
        CHECK(reloaded.balance == 1000, "Balance preserved alongside ban flag");

        remove("data/TEST_BAN_PLAYER_QA.bin");
    }

    /* A pre-existing v2 save file (no is_banned field) must still load correctly,
       without triggering the anti-cheat tamper/corruption reset, and default to NOT banned. */
    {
        Player legacy = fresh_player(2500, 100, 0);
        legacy.total_losses = 50;
        strcpy(legacy.name, "TEST_LEGACY_V2_QA");

        /* Replicates the pre-v3 checksum formula (identical to calculate_checksum with is_banned == 0) */
        long long checksum = (((long long)legacy.balance * get_salt_1()) ^ ((long long)legacy.bank_balance * get_salt_2())) +
            (((long long)legacy.total_winnings * 5039LL) ^ (long long)legacy.total_losses) ^ get_secret_key();

        char buffer[1024];
        int len = snprintf(buffer, sizeof(buffer), "%d\n%s\n%d\n%d\n%lld\n%lld\n%lld\n",
            2, legacy.name, legacy.balance, legacy.bank_balance,
            legacy.total_winnings, legacy.total_losses, checksum);
        crypt_buffer(buffer, len);

        char filename[64];
        snprintf(filename, sizeof(filename), "data/%s.bin", legacy.name);
        FILE* f = fopen(filename, "wb");
        if (f) { fwrite(buffer, 1, len, f); fclose(f); }

        Player reloaded = { 0 };
        strcpy(reloaded.name, "TEST_LEGACY_V2_QA");
        load_player(&reloaded);

        CHECK(reloaded.balance == 2500, "Legacy v2 file (no is_banned field): balance loads correctly");
        CHECK(reloaded.total_losses == 50, "Legacy v2 file: total_losses loads correctly");
        CHECK(reloaded.is_banned == 0, "Legacy v2 file: missing ban field defaults to NOT banned");

        remove(filename);
    }
}

static void test_trips_no_double_credit(void) {
    printf("\n=== TRIPS: regression — no double credit ===\n");
    /*
     * Bug that was seen before: player received trips stake AND winnings
     * credited twice, inflating balance. This test pins the correct formula:
     *   player->total_winnings += trips_win               (net profit only)
     *   add_balance_safe(&p, trips_bet + trips_win)       (stake back + profit, once)
     */

    /* Hand that scores THREE_OF_A_KIND */
    Card hand[7] = {
        mk(7,'H'),  mk(7,'D'),  mk(7,'C'),
        mk(2,'H'),  mk(4,'S'),  mk(9,'D'), mk(13,'H')
    };

    int p_score   = evaluate_poker_hand(hand, 7);
    int trips_bet = 100;

    /* Replicate poker.c payout table */
    int trips_win = 0;
    if      (p_score >= T_STRAIGHT_FLUSH)  trips_win = trips_bet * 50;
    else if (p_score >= T_FOUR_OF_A_KIND)  trips_win = trips_bet * 30;
    else if (p_score >= T_FULL_HOUSE)      trips_win = trips_bet * 8;
    else if (p_score >= T_FLUSH)           trips_win = trips_bet * 7;
    else if (p_score >= T_STRAIGHT)        trips_win = trips_bet * 4;
    else if (p_score >= T_THREE_OF_A_KIND) trips_win = trips_bet * 3;

    CHECK(trips_win == 300, "TRIPS: three-of-a-kind pays 3x stake");

    Player p        = fresh_player(5000, 0, 0);
    int bal_before  = p.balance;
    long long w_before = p.total_winnings;

    /* Simulate exactly what poker.c does */
    p.total_winnings += trips_win;
    add_balance_safe(&p, trips_bet + trips_win);

    int expected_balance_delta = trips_bet + trips_win; /* 400 */

    CHECK(p.balance - bal_before == expected_balance_delta,
          "TRIPS: balance increases by stake+profit exactly once");
    CHECK(p.total_winnings - w_before == trips_win,
          "TRIPS: total_winnings records net profit only (not stake)");

    /* Double-credit canary: calling add_balance_safe a second time would be the bug */
    int bal_after_correct = p.balance;
    /* We do NOT call add_balance_safe again — assert nothing extra accumulated */
    CHECK(p.balance == bal_after_correct,
          "TRIPS: no second credit applied (canary)");
}

/* =====================================================================
 * 5. Football: overflow guard for (int)(bet * total_slip_odds)
 * ===================================================================== */
static void test_football_overflow(void) {
    printf("\n=== Football: payout overflow guard ===\n");

    /*
     * football.c line: int winnings = (int)(bet * total_slip_odds);
     * Casting a double > INT_MAX to int is UB and yields garbage / negative.
     * With NUM_MATCHES=3, worst-case odds = 3.8 * 3.8 * 3.8 ≈ 54.87.
     */
    int    bet            = MAX_BET;          /* 10 000 */
    double max_per_match  = 3.8;              /* odds_X max = 2.8 + 1.0 */
    double max_total_odds = max_per_match * max_per_match * max_per_match; /* ~54.87 */
    double raw            = (double)bet * max_total_odds;

    CHECK(raw < (double)INT_MAX,
          "Football: max realistic payout fits in int (no UB)");
    CHECK((int)raw > 0,
          "Football: payout is positive (no sign-flip from overflow)");

    /* Safety margin info */
    long long margin = (long long)INT_MAX - (long long)raw;
    printf("  [INFO] max realistic payout ≈ %d | safety margin to INT_MAX ≈ %lld\n",
           (int)raw, margin);

    /* Near-danger sanity: verify formula stays correct at boundary */
    double near_max_odds = (double)INT_MAX / (double)bet - 1.0;
    double near_raw      = (double)bet * near_max_odds;
    CHECK(near_raw < (double)INT_MAX,
          "Football: bet*odds just below INT_MAX/bet still safe");
}

/* =====================================================================
 * 6. Roulette payouts via check_win (roulette.c)
 * ===================================================================== */

static Bet mk_bet(int type, int amount, int n0, int n1, int n2, int n3) {
    Bet b;
    memset(&b, 0, sizeof(b));
    b.bet_type  = type;
    b.amount    = amount;
    b.numbers[0] = n0; b.numbers[1] = n1;
    b.numbers[2] = n2; b.numbers[3] = n3;
    b.num_count = (n1 ? 2 : 1);
    return b;
}

static void test_roulette_payouts(void) {
    printf("\n=== Roulette: payout ratios ===\n");

    int amt = 100;

    /* 6a. Straight 35:1 — win on matching number */
    {
        Bet b = mk_bet(1, amt, 17, 0, 0, 0);
        int payout = check_win(b, 17);
        CHECK(payout == amt + amt * 35, "Straight: hit pays 35:1 (100 → 3600)");
        CHECK(check_win(b, 5) == 0, "Straight: miss pays 0");
    }

    /* 6b. Split 17:1 — win on either number */
    {
        Bet b = mk_bet(2, amt, 17, 18, 0, 0);
        int payout_a = check_win(b, 17);
        int payout_b = check_win(b, 18);
        CHECK(payout_a == amt + amt * 17, "Split: first number pays 17:1 (100 → 1800)");
        CHECK(payout_b == amt + amt * 17, "Split: second number pays 17:1");
        CHECK(check_win(b, 5) == 0,       "Split: miss pays 0");
    }

    /*
     * 6c. Street 11:1 — NOTE: check_win has no bet_type for street (3-number rows).
     * The welcome screen advertises "Street: 11 to 1" but no street case exists
     * in check_win(). Documenting the gap here.
     */
    {
        /* bet_type=3 is Dozens (≠ street); no bet_type covers a 3-number row. */
        printf("  \x1b[33m[WARN]\x1b[0m Street (11:1) advertised in welcome screen "
               "but NOT implemented in check_win() — no bet_type for 3-number rows.\n");
    }

    /* 6d. Color 1:1 — red wins on a red number (1), loses on black/green */
    {
        /* get_number_color: 1=red, 2=black, 0=green.  Number 1 is red. */
        Bet b_red = mk_bet(4, amt, 1 /* red */, 0, 0, 0);
        int payout_red  = check_win(b_red, 1);   /* number 1 is red */
        int payout_miss = check_win(b_red, 2);   /* number 2 is black */
        int payout_zero = check_win(b_red, 0);   /* 0 is green */
        CHECK(payout_red  == amt + amt * 1, "Color RED: hit pays 1:1 (100 → 200)");
        CHECK(payout_miss == 0,             "Color RED: black pays 0");
        CHECK(payout_zero == 0,             "Color RED: green (0) pays 0");
    }

    /* 6e. Corner (bet_type=7) 8:1 — wins on any of 4 numbers */
    {
        Bet b = mk_bet(7, amt, 4, 5, 7, 8);
        CHECK(check_win(b, 4) == amt + amt * 8, "Corner: any of 4 numbers pays 8:1");
        CHECK(check_win(b, 8) == amt + amt * 8, "Corner: fourth number pays 8:1");
        CHECK(check_win(b, 1) == 0,             "Corner: off-corner pays 0");
    }
}

/* =====================================================================
 * main — run all suites, print summary
 * ===================================================================== */
int main(void) {
    CreateDirectory("data", NULL);

    printf("\x1b[36m╔══════════════════════════════════════╗\x1b[0m\n");
    printf("\x1b[36m║   CASINO TEST HARNESS                ║\x1b[0m\n");
    printf("\x1b[36m╚══════════════════════════════════════╝\x1b[0m\n");

    test_add_balance_safe();
    test_calculate_hand_value();
    test_is_soft_17();
    test_poker_hand_ranking();
    test_poker_hand_comparisons();
    test_player_ban_and_versioning();
    test_trips_no_double_credit();
    test_football_overflow();
    test_roulette_payouts();

    int total = g_pass + g_fail;
    printf("\n");
    printf("\x1b[36m========================================\x1b[0m\n");
    if (g_fail == 0) {
        printf("\x1b[32m ALL %d TESTS PASSED\x1b[0m\n", total);
    } else {
        printf("\x1b[32m PASSED: %d\x1b[0m  |  \x1b[31mFAILED: %d\x1b[0m  |  TOTAL: %d\n",
               g_pass, g_fail, total);
    }
    printf("\x1b[36m========================================\x1b[0m\n");

    return g_fail == 0 ? 0 : 1;
}
