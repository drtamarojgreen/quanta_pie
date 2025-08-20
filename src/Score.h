#ifndef SCORE_H
#define SCORE_H

class Score {
public:
    Score(int id, int p_id, int s_id, int val)
        : score_id(id), player_id(p_id), session_id(s_id), score(val) {}

    int getScoreID() const { return score_id; }
    int getPlayerID() const { return player_id; }
    int getSessionID() const { return session_id; }
    int getScoreValue() const { return score; }

private:
    int score_id;
    int player_id;
    int session_id;
    int score;
};

#endif // SCORE_H
