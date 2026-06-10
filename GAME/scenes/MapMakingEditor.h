#pragma once
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "Lighting\Light.h"
#include "Camera\DebugCamera.h"
#include "GAME\Object\DetailButton\DetailButton.h"
#include "GameObject\button\button.h"
#include "Tool\DefaultMenu\DefaultMenu.h"

class MapMakingEditor: public BaseScene
{
public:
    MapMakingEditor(kEngine* system);
    ~MapMakingEditor();

    void Update() override;
    void Draw() override;

private:
    




};

