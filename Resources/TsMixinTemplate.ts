import * as UE from "ue"
import {argv} from "puerts"
import { $Ref,$ref,$set,$unref, blueprint } from "puerts";

let gameInstance = argv.getByName("GameInstance") as %GAME_INSTANCE_TYPE%;
let BP_MixinClass = UE.Class.Load('%BLUEPRINT_PATH%');
const %BP_TO_JS% = blueprint.tojs<typeof %MIXIN_BLUEPRINT_TYPE%>(BP_MixinClass);
interface %TS_CLASS_NAME% extends %MIXIN_BLUEPRINT_TYPE%{};

class %TS_CLASS_NAME% {
    ReceivebeginPlay():void{
        console.warn("%TS_CLASS_NAME%::ReceivebeginPlay()"); 
    }

    TSFunction_E():void{
        console.log("%TS_CLASS_NAME%::TSFunction_E()");
    }
}
    
blueprint.mixin(%BP_TO_JS%,%TS_CLASS_NAME%)