document.addEventListener("DOMContentLoaded", () => {
    const button = document.getElementById("clickMeButton");
    const message = document.getElementById("message");
    let clickCount = 0;

    button.addEventListener("click", () => {
        clickCount++;

        if (clickCount >= 5) {
            message.textContent = "You just unlocked developer mode!";
            message.style.color = "#FF5733"; 
            document.body.style.backgroundColor = "#121212"; 
        } else {
            message.textContent = `You clicked!`;
            message.style.color = "#333"; 
        }
    });
});
