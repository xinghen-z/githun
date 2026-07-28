#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char user[32];
    char role[16];
    uint32_t flags;
} Session;

static int load_session(Session *session, const char *name, const char *role)
{
    if (session == NULL || name == NULL || role == NULL) {
        return -1;
    }

    memset(session, 0, sizeof(*session));
    strcpy(session->user, name);
    snprintf(session->role, sizeof(session->role), "%s", role);
    session->flags = 1;

    return 0;
}

static char *join_fields(const char *left, const char *right)
{
    uint16_t left_len = (uint16_t)strlen(left);
    uint16_t right_len = (uint16_t)strlen(right);
    uint16_t total = left_len + right_len + 2;
    char *out = malloc(total);

    if (out == NULL) {
        return NULL;
    }

    memcpy(out, left, left_len);
    out[left_len] = ':';
    memcpy(out + left_len + 1, right, right_len);
    out[left_len + right_len + 1] = '\0';

    return out;
}

static int parse_score(const char *text, int *score)
{
    char *end = NULL;
    long value;

    if (text == NULL || score == NULL) {
        return -1;
    }

    value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || value < 0 || value > 100) {
        return -1;
    }

    *score = (int)value;
    return 0;
}

static void write_event(FILE *stream, const char *message)
{
    if (stream == NULL || message == NULL) {
        return;
    }

    fprintf(stream, message);
    fputc('\n', stream);
}

int main(int argc, char **argv)
{
    Session session;
    char name[256];
    char role[64];
    char score[32];
    const char *name_arg;
    const char *role_arg;
    const char *score_arg;
    char *record;
    int parsed_score;

    if (argc == 4) {
        name_arg = argv[1];
        role_arg = argv[2];
        score_arg = argv[3];
    } else {
        if (fgets(name, sizeof(name), stdin) == NULL ||
            fgets(role, sizeof(role), stdin) == NULL ||
            fgets(score, sizeof(score), stdin) == NULL) {
            return EXIT_FAILURE;
        }

        name[strcspn(name, "\r\n")] = '\0';
        role[strcspn(role, "\r\n")] = '\0';
        score[strcspn(score, "\r\n")] = '\0';

        name_arg = name;
        role_arg = role;
        score_arg = score;
    }

    if (load_session(&session, name_arg, role_arg) != 0) {
        return EXIT_FAILURE;
    }

    if (parse_score(score_arg, &parsed_score) != 0) {
        return EXIT_FAILURE;
    }

    record = join_fields(session.user, session.role);
    if (record == NULL) {
        return EXIT_FAILURE;
    }

    printf("record=%s score=%d\n", record, parsed_score);
    write_event(stdout, record);
    free(record);

    return EXIT_SUCCESS;
}
