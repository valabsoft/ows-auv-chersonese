#ifndef SEVROVCONTROLLER_H
#define SEVROVCONTROLLER_H

enum class ViewMode { SINGLE, STEREO };

class SevROVController
{
public:
    SevROVController();
    bool isConnected = false;
    ViewMode viewMode = ViewMode::SINGLE;
};

#endif // SEVROVCONTROLLER_H
