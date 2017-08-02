var addon = require('../bin/F22U.node');
var finger = new addon.F22U();

exports.open = function (realCallback, errorCallback) {
    return finger.Open(realCallback, errorCallback);
};

exports.close = function () {
    return finger.Close();
};

exports.start = function (featureTimes, imageScores, featurePoints, encrollScores) {
    return finger.Start(featureTimes, imageScores, featurePoints, encrollScores);
};

exports.pause = function () {
    return finger.Pause();
};