### mail client https://app.mailgun.com/ angrycans@gmail.com free 5k/mo

### dropbox accesstoken 
xlap
sl.BYA-5L2BntvvHNSX-zzb29druYkj8W2rRq6AjbQ89CzU-Vp54TGKcP2idgU3QJl1qElofNLYAy2_4AIIlSq8mOdL00QOESYcJ4U4iSDLxvTyhMJU6jQjc2rXTzxoahxECjNLQicMgAUc


curl -X POST https://api.dropboxapi.com/2/files/list_folder \
    --header "Authorization: Bearer sl.BYA-5L2BntvvHNSX-zzb29druYkj8W2rRq6AjbQ89CzU-Vp54TGKcP2idgU3QJl1qElofNLYAy2_4AIIlSq8mOdL00QOESYcJ4U4iSDLxvTyhMJU6jQjc2rXTzxoahxECjNLQicMgAUc" \
    --header "Content-Type: application/json" \
    --data "{\"include_deleted\":false,\"include_has_explicit_shared_members\":false,\"include_media_info\":false,\"include_mounted_folders\":true,\"include_non_downloadable_files\":true,\"path\":\"/angrycans@gmail.com\",\"recursive\":false}"



curl -X POST "https://api.dropboxapi.com/2/users/get_current_account" \
    --header "Authorization: Bearer sl.BYA-5L2BntvvHNSX-zzb29druYkj8W2rRq6AjbQ89CzU-Vp54TGKcP2idgU3QJl1qElofNLYAy2_4AIIlSq8mOdL00QOESYcJ4U4iSDLxvTyhMJU6jQjc2rXTzxoahxECjNLQicMgAUc"


curl -X POST https://api.dropboxapi.com/2/file_requests/list_v2 \
    --header "Authorization: Bearer sl.BYA-5L2BntvvHNSX-zzb29druYkj8W2rRq6AjbQ89CzU-Vp54TGKcP2idgU3QJl1qElofNLYAy2_4AIIlSq8mOdL00QOESYcJ4U4iSDLxvTyhMJU6jQjc2rXTzxoahxECjNLQicMgAUc" \
    --header "Content-Type: application/json" \
    --data "{\"limit\":100}"


curl -X POST https://api.dropboxapi.com/2/files/create_folder_v2 \
    --header "Authorization: Bearer sl.BYA-5L2BntvvHNSX-zzb29druYkj8W2rRq6AjbQ89CzU-Vp54TGKcP2idgU3QJl1qElofNLYAy2_4AIIlSq8mOdL00QOESYcJ4U4iSDLxvTyhMJU6jQjc2rXTzxoahxECjNLQicMgAUc" \
    --header "Content-Type: application/json" \
    --data "{\"autorename\":false,\"path\":\"/angrycans@gmail.com\"}"


curl -X POST https://api.dropboxapi.com/2/file_requests/get \
    --header "Authorization: Bearer sl.BYA-5L2BntvvHNSX-zzb29druYkj8W2rRq6AjbQ89CzU-Vp54TGKcP2idgU3QJl1qElofNLYAy2_4AIIlSq8mOdL00QOESYcJ4U4iSDLxvTyhMJU6jQjc2rXTzxoahxECjNLQicMgAUc" \
    --header "Content-Type: application/json" \
    --data '{"id":"zM3YGUwBmyMAAAAAAAAADA"}'


curl -X POST "https://api.dropboxapi.com/2/users/get_current_account" \
    --header "Authorization: Bearer sl.BYIt3TU1uLysKTaHUAAxfSuj27TOcye777y2FHyadDRcUFxA6awHBUuMPq2YXoY4lCU8zQgcXBFbnkMALpgQGRnN1ZxQZd9_sqlHGK7DqfXzhAph9L1mAP4cTEGI51_9z6wcepmBf7df"


请求 code 一次 
https://www.dropbox.com/oauth2/authorize?client_id=054fu81io7bpqjm&token_access_type=offline&response_type=code

请求refresh_token
curl --location --request POST 'https://api.dropboxapi.com/oauth2/token' \
-u '054fu81io7bpqjm:fkjvl5t718xyj16' \
-H 'Content-Type: application/x-www-form-urlencoded' \
--data-urlencode 'code=IC3cIz6dSiAAAAAAAAAAktJDYB0GrrHlBgtgCVeqtrY' \
--data-urlencode 'grant_type=authorization_code'


refresh_token 
需要使用 refresh_token 刷新accesstoken

curl -v --trace-ascii curl.trace https://api.dropbox.com/oauth2/token \
   -d refresh_token=7n_Dt3I2izUAAAAAAAAAAbno9CplSr8CcdFYeMkA-1-Uuf-FX2fWks0ep8Xx7npL \
   -d grant_type=refresh_token \
   -d client_id=054fu81io7bpqjm \
   -d client_secret=fkjvl5t718xyj16 

 curl -v https://api.dropbox.com/oauth2/token \
   -d refresh_token=7n_Dt3I2izUAAAAAAAAAAbno9CplSr8CcdFYeMkA-1-Uuf-FX2fWks0ep8Xx7npL \
   -d grant_type=refresh_token \
   -d client_id=054fu81io7bpqjm \
   -d client_secret=fkjvl5t718xyj16   



   curl https://api.dropbox.com/oauth2/token?client_id=054fu81io7bpqjm&client_secret=fkjvl5t718xyj16&grant_type=refresh_token&refresh_token=7n_Dt3I2izUAAAAAAAAAAbno9CplSr8CcdFYeMkA-1-Uuf-FX2fWks0ep8Xx7npL


