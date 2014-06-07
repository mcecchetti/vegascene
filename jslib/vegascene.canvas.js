
var require = null;
vegascene_template = function(vs, context2d) {
    require = function(module) {
        if ( module === 'canvas')
            return vs.canvas;
        else
            console.log('warning: `require` called with an unknown module: ' + module);
    }

    vs.canvas = function() {
    };

    vs.canvas.prototype.getContext = function(ctxType) {
        if (ctxType === '2d')
            return vs[context2d];
        else
            console.log('warning: `canvas.getContext` called with an unknown context type: ' + ctxType);
    };
};
