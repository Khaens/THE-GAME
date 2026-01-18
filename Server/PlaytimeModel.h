#pragma once

class PlaytimeModel {
private:
    int id = 0;
    int userId = 0;
    int seconds = 0;

public:
    PlaytimeModel() = default;
    PlaytimeModel(int uid); 

    int GetId() const;
    int GetUserId() const;
    int GetSeconds() const;

    void SetId(int v);
    void SetUserId(int v);
    void SetSeconds(int v);

    float GetTotalHours() const; 
};