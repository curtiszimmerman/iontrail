load(libdir + "parallelarray-helpers.js");

function buildSimple() {

    let obj = { x: 1, y: 2, z: 3 };

    new ParallelArray([256], function(i) {
        obj.x += 1;
        obj.y += 1;
        obj.z += 1;
        return obj;
    }, {mode: "par", expect: "bail"});

}

buildSimple();
