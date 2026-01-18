#include "PlaytimeModel.h"

PlaytimeModel::PlaytimeModel(int uid) : userId(uid), seconds(0) 
{
}

int PlaytimeModel::GetId() const { return id; }
int PlaytimeModel::GetUserId() const { return userId; }
int PlaytimeModel::GetSeconds() const { return seconds; }

void PlaytimeModel::SetId(int v) { id = v; }
void PlaytimeModel::SetUserId(int v) { userId = v; }
void PlaytimeModel::SetSeconds(int v) { seconds = v; }

float PlaytimeModel::GetTotalHours() const {
    return static_cast<float>(seconds) / 3600.0f;
}