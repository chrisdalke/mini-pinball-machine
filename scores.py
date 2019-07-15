import minidb

class Score(minidb.Model):
    name = str
    score = int

db = None
def initScores():
    global db
    db = minidb.Store('Resources/scores.db', debug=True)
    db.register(Score)

def registerScore(name,score):
    global db
    tempScore = Score(name=name,score=score)
    tempScore.save(db)
    db.commit()

def topTen():
    global db
    return list(Score.query(db, Score.c.name // Score.c.score, order_by=Score.c.score.desc, limit=10))

if __name__ == "__main__":
    initScores()
    registerScore("dalk",345)
    print(topTen())
