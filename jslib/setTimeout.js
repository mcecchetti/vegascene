
var setTimeout = undefined;
vegascene_template = function(timeoutBackend) {
    setTimeout = function (callback, delay) {
        timeoutBackend.Start(callback, delay);
    }
}
