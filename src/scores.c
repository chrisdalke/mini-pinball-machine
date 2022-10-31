#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "scores.h"

ScoreHelper *initScores(){
    ScoreHelper *helper = malloc(sizeof(ScoreHelper));
    helper->scoresUpdated = 0;
    helper->numTopScores = 0;
    helper->scores = malloc(sizeof(ScoreObject) * 10);
    helper->initialized = 0;

    int rc = sqlite3_open("Resources/scores.db", &helper->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(helper->db));
        sqlite3_close(helper->db);
    } else {
        helper->initialized = 1;
        // Create sqlite table if it does not exist.
        char *sql = "CREATE TABLE IF NOT EXISTS Score (id INTEGER PRIMARY KEY, name TEXT, score INTEGER);";
        rc = sqlite3_exec(helper->db, sql, NULL, NULL, NULL);

        // Blank scores with null score objects.
        for (int i = 0; i < 10; i++){
            helper->scores[i].scoreName = (char*) malloc(12*sizeof(char));
            helper->scores[i].scoreValue = 0;
        }
        // As a test, show top 10 scores
        for (int i = 1; i <= 10; i++){
            ScoreObject *score = getRankedScore(helper,i);
            if (score != NULL){
                printf("%d %s = %d\n",i,score->scoreName,score->scoreValue);
            } else {
                printf("%d NULL\n",i);
            }
        }
    }
    return helper;
}

void shutdownScores(ScoreHelper *helper){
    if (helper->initialized == 1){
        sqlite3_close(helper->db);
        helper->initialized = 0;
    }
}
void submitScore(ScoreHelper *helper, char *name, int score){
    if (helper->initialized == 1){
        // invalidate scores
        helper->scoresUpdated = 0;
        int rc = sqlite3_prepare_v2(helper->db, "insert into Score(name,score) values (?,?)", -1, &helper->insertStatement, NULL);
        printf("insert: %d\n",rc);
        sqlite3_bind_text(helper->insertStatement, 1, name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(helper->insertStatement, 2, score);
        rc = sqlite3_step(helper->insertStatement);
        sqlite3_finalize(helper->insertStatement);
        printf("submitted score status = %d\n",rc);
    }
}

ScoreObject *getRankedScore(ScoreHelper *helper, int rank){
    if (helper->initialized == 1){
        // if score table is invalid, update it.
        if (helper->scoresUpdated == 0){
            char *sql1 = "drop table if exists SortedScores;";
            char *sql2 = "create table SortedScores as select name, score from Score ORDER BY score DESC LIMIT 10;";
            char *sql3 = "select rowid, name, score from SortedScores order by rowid;";
            printf("Loading score table\n");
            helper->numTopScores = 0;
            int rc1 = sqlite3_exec(helper->db, sql1, NULL, NULL, NULL);
            int rc2 = sqlite3_exec(helper->db, sql2, NULL, NULL, NULL);
            int rc3 = sqlite3_exec(helper->db, sql3, sqlCallback, helper, NULL);
            printf("%d %d %d\n",rc1,rc2,rc3);
            helper->scoresUpdated = 1;
        }
        // Check that number range is valid.
        if (rank > 0 && rank <= helper->numTopScores){
            return &helper->scores[rank - 1];
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

// Called for each row in the score query
int sqlCallback(void *helper, int argc, char **argv, char **azColName){
    ScoreHelper *tempHelper = (ScoreHelper*)helper;
    char *rowId = argv[0];
    int rowIdInt = atoi(rowId) - 1;
    char *scoreName = argv[1];
    char *scoreValue = argv[2];
    printf("%d %s %s \n",rowIdInt, scoreName, scoreValue);
    if (rowIdInt >= 0 && rowIdInt < 10){
        strcpy(tempHelper->scores[rowIdInt].scoreName, scoreName);
        int j = 0;
        while (tempHelper->scores[rowIdInt].scoreName[j]) {
            tempHelper->scores[rowIdInt].scoreName[j] = toupper(tempHelper->scores[rowIdInt].scoreName[j]);
            j++;
        }
        tempHelper->scores[rowIdInt].scoreValue = atoi(scoreValue);
        tempHelper->numTopScores++;
    }
    return 0;
}
