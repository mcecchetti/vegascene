
var console = {};

vegascene_template = function(consoleBackend) {
    console.log = consoleBackend.Log;
    console.view = consoleBackend.View;
}
