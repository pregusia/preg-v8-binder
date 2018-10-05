
var obj = new testing.TestClass(4343);


print('test1');
print(obj.value);
obj.add(12);
print(obj.value);
obj.value = 123;
print(obj.asString());

print(testGlobalFunc(10,20));

// bind some event
var someObject = getSomeObjectInstance();
$(someObject).bind('onTestEvent', function(){
	print('test event called');
});

