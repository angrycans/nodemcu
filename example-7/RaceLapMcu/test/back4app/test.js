var API_KEY = "0c59ad5869a35dde3548aaeffc2dbda4-75cd784d-e7c8d672";
var DOMAIN = "sandboxcb01f9c5709e44948318d103118cbb2e.mailgun.org";
var mailgun = require("mailgun-js")({ apiKey: API_KEY, domain: DOMAIN });

(async () => {
  console.log("start");
  var sender_email = "angrycans@gmail.com";
  var receiver_email = "angrycans@gmail.com";
  var email_subject = "Mailgun Demo";
  var email_body = "Greetings from geeksforgeeks";

  const data = {
    "from": sender_email,
    "to": receiver_email,
    "subject": email_subject,
    "text": email_body,
  };

  const res = await mailgun.messages().send(data);
  console.log(res);
})();
