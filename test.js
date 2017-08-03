try {
	var fs = require('fs');
	var finger = require('./lib/F22U');
	var result = finger.open(function (fingerData) {
		if (fingerData.type == 1) {
			var imageBuffer = new Buffer(fingerData.data, 'base64');
			fs.writeFile("finger_image.bmp", imageBuffer, function (err) {
				if (err) {
					console.log('保存指纹图片失败');
				}
			});
		}
		if (fingerData.type == 2) {
			console.info('指纹模板信息:' + fingerData.data);
		}
	}, function (err) {})
	console.log('初始化读卡设备：', result > 0 ? '成功' : '失败');

	result = finger.start(2, 75, 15, 100);
	console.log('开启读取指纹：', result > 0 ? '成功' : '失败');
	
	setTimeout(function () {
		result = finger.pause();
		console.log('停止读取指纹：', result > 0 ? '成功' : '失败');
	}, 20000);
} catch (error) {
	console.info(error);
}

process.on('uncaughtException', function (err) {
	console.log(err);
});

process.stdin.resume();