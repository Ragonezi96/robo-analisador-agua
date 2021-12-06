// MQTT subscriber
var mqtt = require('mqtt')
var client = mqtt.connect('mqtt://localhost:1234')
var topic = 'sensor_tds'
var mysql = require('mysql');

client.on('message', (topic, message)=>{
    message = message.toString()
    var today = new Date();
    var date = today.getFullYear()+'-'+(today.getMonth()+1)+'-'+today.getDate();
    var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
    var dateTime = date+' '+time;
    let query = "INSERT INTO monitored_data.tds (tds, `data`) VALUES('"+message+"', '"+dateTime+"')"

    //create connection
    var con = mysql.createConnection({
      host: "database-1.c6h2jt1jsshs.us-east-2.rds.amazonaws.com",
      user: "admin",
      password: "teste123",
      database: "monitored_data"
    });
    con.connect(function(err) {
        if (err) throw err;
        con.query(query, function (err, result, fields) {
          if (err) throw err;
          console.log(result);
        });
      });
})

client.on('connect', ()=>{
    client.subscribe(topic)
})
