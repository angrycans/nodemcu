

async function queryBoardName() {
    if (!port || !writer) return;

    console.log("queryBoardName start");

    // 发送 HELLO 指令
    send("HELLO");

    try {
        const boardNameRaw = await waitForLinePrefix("BOARD:", 1200);
        let boardName = "Unknown";
        if (boardNameRaw.startsWith("BOARD:")) {
            boardName = boardNameRaw.split(":")[1];
        }

        document.getElementById("status").textContent = `已连接 · ${boardName}`;
    } catch (err) {
        console.error("读取板子名称失败:", err);
        document.getElementById("status").textContent = "已连接 · Unknown";
    }
}
