/* ff.js
 * Default JavaScript file for https://forwarderfactory.com
 * Licensed under the MIT license
 * Copyright (c) 2025 Jacob Nilsson
 */

function include(file) {
    const script = document.createElement('script');
    script.src = file;
    script.type = 'text/javascript';
    script.defer = true;

    document.getElementsByTagName('head').item(0).appendChild(script);
}

function get_cookie(name) {
    const name_eq = `${name}=`;
    const ca = document.cookie.split(';');
    for (let i = 0; i < ca.length; i++) {
        let c = ca[i];
        while (c.charAt(0) === ' ') {
            c = c.substring(1, c.length);
        }
        if (c.indexOf(name_eq) === 0) {
            return c.substring(name_eq.length, c.length);
        }
    }
    return null;
}

function get_path() {
    return window.location.pathname;
}

function set_path(path) {
    window.history.pushState({}, '', path);
}

function cookie_exists(name) {
    return get_cookie(name) !== null;
}

function WSCBackgroundRepeatingSpawner(speed = 0.5, creation_interval = 8000) {
    const spawnCount = 10; /* hacky as fuck */

    const images = document.querySelectorAll(`img.background-image`);
    images.forEach(img => {
        img.remove();
    });

    const cached_image = new Image();
    cached_image.src = '/img/background-logo-1.png';

    function create_img(topOffset, rightOffset) {
        const img = document.createElement('img');

        img.className = "background-image";
        img.style.position = 'absolute';
        img.style.top = `${topOffset}px`;
        img.style.right = `${rightOffset}px`;
        img.style.opacity = '0.2';
        img.style.zIndex = '-9999';
        img.style.filter = `hue-rotate(${Math.random() * 360}deg)`;
        img.src = cached_image.src;

        img.style.userSelect = 'none';
        img.draggable = false;

        document.body.appendChild(img);

        const move_image = () => {
            const rect = img.getBoundingClientRect();
            if (rect.top > window.innerHeight || rect.left < -rect.width) {
                img.remove();
                return;
            }
            img.style.top = `${parseFloat(img.style.top) + speed}px`;
            img.style.right = `${parseFloat(img.style.right) + speed}px`;

            requestAnimationFrame(move_image);
        };

        move_image();
    }

    const image_spawner = (topOffset, rightOffset) => {
        const horizontalSpacing = 400;
        const numberOfColumns = Math.floor((window.innerWidth / horizontalSpacing) * 2);

        for (let col = 0; col < numberOfColumns; col++) {
            create_img(topOffset, rightOffset + col * horizontalSpacing - 500);
        }
    };

    // HACK: This is a hack to create multiple batches of images initially
    // Ideally, we should use only the setInterval function below
    // But I am a javascript n00b so this will do until someone fixes it, which is probably never anyway
    // It's fine unless you're running this on a penium 4, then you might see some lag
    // and a minor nuclear explosion in your parents' basement
    for (let i = 0; i < spawnCount; i++) {
        const topOffset = -200 + i * speed * creation_interval / 30;
        const rightOffset = -500 + i * speed * creation_interval / 30;
        image_spawner(topOffset, rightOffset);
    }

    // on resize, remove all images and re-spawn them
    window.onresize = () => {
        WSCBackgroundRepeatingSpawner(speed, creation_interval);
    }

    // if we switch tabs, restart
    document.addEventListener('visibilitychange', () => {
        if (document.visibilityState === 'visible') {
            WSCBackgroundRepeatingSpawner(speed, creation_interval);
        }
    });

    setInterval(() => {
        image_spawner(-200, -500);
    }, creation_interval);
}

const click_object = new Audio('/audio/click.wav');
function play_click() {
    click_object.currentTime = 0;
    click_object.play();
}

function hide_all_windows() {
    const windows = document.getElementsByClassName('floating_window');
    for (let i = 0; i < windows.length; i++) {
        windows[i].style.display = 'none';
        while (windows[i].firstChild) {
            windows[i].removeChild(windows[i].firstChild);
        }
    }

    const grids = document.getElementsByClassName('grid');
    for (let i = 0; i < grids.length; i++) {
        grids[i].style.display = 'flex';
    }

    // hide #browse-search and #browse-filter-button if they exist
    const search = document.getElementById('browse-search');
    if (search) {
        search.style.display = 'none';
    }
    const filter = document.getElementById('browse-filter-button');
    if (filter) {
        filter.style.display = 'none';
    }

    // show title if hidden
    const title = document.getElementById('page-header');
    if (title) {
        title.style.display = 'block';
    }

    // hide blacken
    const blacken = document.getElementById('blacken');
    if (blacken) {
        blacken.style.display = 'none';
    }

    set_path("/");
}

function hide_initial() {
    const windows = document.getElementsByClassName('floating_window');
    for (let i = 0; i < windows.length; i++) {
        windows[i].style.display = 'none';
        while (windows[i].firstChild) {
            windows[i].removeChild(windows[i].firstChild);
        }
    }

    const grids = document.getElementsByClassName('grid');
    for (let i = 0; i < grids.length; i++) {
        grids[i].style.display = 'none';
    }

    const search = document.getElementById('browse-search');
    if (search) {
        search.style.display = 'none';
    }
    const filter = document.getElementById('browse-filter-button');
    if (filter) {
        filter.style.display = 'none';
    }

    const title = document.getElementById('page-header');
    if (title) {
        title.style.display = 'block';
    }

    const blacken = document.getElementById('blacken');
    if (blacken) {
        blacken.style.display = 'none';
    }
}

class WindowProperties {
    constructor({
        back_button = null,
        close_button = true,
        moveable = false,
        close_on_click_outside = false,
        close_on_escape = true,
        remove_existing = true,
        function_on_close = null
    } = {}) {
        this.back_button = back_button;
        this.close_button = close_button;
        this.moveable = moveable;
        this.close_on_click_outside = close_on_click_outside;
        this.close_on_escape = close_on_escape;
        this.remove_existing = remove_existing;
        this.function_on_close = function_on_close;
    }
}

function create_window(id, prop = new WindowProperties()){
    if (prop.remove_existing) {
        const windows = document.getElementsByClassName('floating_window');
        for (let i = 0; i < windows.length; i++) {
            windows[i].style.display = 'none';
            while (windows[i].firstChild) {
                windows[i].removeChild(windows[i].firstChild);
            }
        }
    }
    // remove existing with same id always
    const existing = document.getElementById(id);
    if (existing) {
        existing.style.display = 'none';
        while (existing.firstChild) {
            existing.removeChild(existing.firstChild);
        }
    }
    const window = document.createElement('div');
    window.className = 'floating_window';
    window.id = id;
    if (prop.close_on_click_outside) {
        window.onclick = (event) => {
            if (event.target === window) {
                if (prop.function_on_close) {
                    prop.function_on_close();
                    return;
                }
                hide_all_windows();
            }
        }
    }
    if (prop.close_on_escape) {
        document.onkeydown = (event) => {
            if (event.key === 'Escape') {
                if (prop.function_on_close) {
                    prop.function_on_close();
                    return;
                }
                hide_all_windows();
            }
        }
    }

    window.oncontextmenu = (event) => {
        event.preventDefault();
    }

    let xpos = 0;
    let ypos = 0;
    let offsetX = 0;
    let offsetY = 0;

    if (prop.moveable) {
        window.onmousedown = (event) => {
            xpos = event.clientX;
            ypos = event.clientY;
            offsetX = xpos - window.offsetLeft;
            offsetY = ypos - window.offsetTop;

            document.onmousemove = (event) => {
                window.style.left = (event.clientX - offsetX) + 'px';
                window.style.top = (event.clientY - offsetY) + 'px';
            }
        }

        document.onmouseup = () => {
            document.onmousemove = null;
        }
    }

    if (prop.close_button) {
        const close = document.createElement('a');
        close.innerHTML = '✕';
        close.id = 'window-close';
        close.style.position = 'fixed';
        close.style.padding = '10px';
        close.style.top = '0';
        close.style.right = '0';
        close.style.textDecoration = 'none';
        close.style.color = 'black';
        close.onclick = () => {
            play_click();
            if (prop.function_on_close) {
                prop.function_on_close();
                return;
            }
            hide_all_windows();
        }

        window.appendChild(close);
    }
    if (prop.back_button) {
        const back = document.createElement('a');
        back.innerHTML = '←';
        back.id = 'window-back';
        back.style.position = 'fixed';
        back.style.padding = '10px';
        back.style.top = '0';
        back.style.left = '0';
        back.style.textDecoration = 'none';
        back.style.color = 'black';
        back.onclick = () => {
            play_click();
            if (prop.function_on_close) {
                prop.function_on_close();
            }
        }

        window.appendChild(back);
    }

    document.body.appendChild(window);

    return window;
}

function show_terms() {
    play_click();
    const terms = create_window('terms-window');

    const title = document.createElement('h1');
    title.innerHTML = 'Terms of Service';

    const paragraph = document.createElement('p');
    let terms_str = "By contributing, logging in and/or registering to this website (hereinafter referred to as 'Forwarder Factory'), you agree to the following terms of service:";
    terms_str += "<br>1. Forwarder Factory, its contributors and its members shall not be held responsible for any damages caused by the use of files, software, information, or any other content found on this website.";
    terms_str += "<br>2. Forwarder Factory reserves the right to remove any content at any time for any reason, including but not limited to, a DMCA takedown request.";
    terms_str += "<br>3. Forwarder Factory reserves the right to ban users who upload harmful, malicious or otherwise dangerous content.";
    terms_str += "<br>4. Forwarder Factory reserves the right to change these terms at any time without notice.";
    terms_str += "<br>5. Forwarder Factory cannot guarantee it will keep your data safe, and it shall not be held responsible for any data breaches. We therefore highly urge our users not to use the same password on Forwarder Factory as they do on other websites. Data breaches are not expected, but they are possible, and we want to be transparent about it.";
    terms_str += "<br>6. Any data you submit to Forwarder Factory may be stored indefinitely, until a request for deletion is received. Forwarder Factory will not sell your data to third parties.";
    terms_str += " In the event of a data breach, we will attempt to notify all affected users as soon as possible, within reason.";
    terms_str += "<br>European Union: By using this website, you agree to the use of cookies. We only use cookies for session management, not for tracking or advertising purposes. You have the right to request that your data be deleted at any time, and we will comply with any such requests. Send requests in the form of an email to contact@forwarderfactory.com.";
    terms_str += "<br>Note: Forwarder Factory is not affiliated with Nintendo. All trademarks are property of their respective owners.";
    terms_str += "<br>Forwarder Factory is hosted in Sweden, and is therefore subject to Swedish law. Uploads that do not comply with Swedish law will be removed on sight, as we are legally required to do so.";

    paragraph.innerHTML = terms_str;

    terms.appendChild(title);
    terms.appendChild(paragraph);
}

function show_login(_error = "") {
    play_click();
    set_path('/login');

    hide_initial();

    /* username/email, password */
    let ret = {};

    const submit_data = () => {
        play_click();

        const json = {
            password: ret.password.value,
        };
        if (ret.username.value.includes('@')) {
            json.email = ret.username.value;
        } else {
            json.username = ret.username.value;
        }

        fetch("/api/try_login", {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(json),
        })
            .then(response => response.json())
            .then(json => {
                if (json.error_str) {
                    show_login(json.error_str);
                    return;
                } else if (json.key) {
                    // a session entry has now been created, this was done by the server so
                    // we don't need to do anything.
                    // however, we have a key, though it's only useful if we can't use
                    // cookies for some reason (e.g. if we're using a different device)
                    // let's redir to the home page
                    window.location.href = '/';
                    return;
                }

                throw new Error('Invalid response from server: ' + JSON.stringify(json));
            })
            /*
            .then(data => {
                hide_all_windows();
            })
            */
            .catch((error) => {
                console.error('Error:', error);
            });
    }

    const ask_for_password = () => {
        play_click();

        const login = create_window('login-window');

        const password = document.createElement('input');
        password.type = 'password';
        password.name = 'password';
        password.placeholder = 'Password';
        password.className = 'login-input';
        password.id = 'login-password';
        password.onclick = () => {
            play_click();
            const errors = document.getElementsByClassName('error');
            for (let i = 0; i < errors.length; i++) {
                const error = errors[i];
                if (error.id === 'login-error') {
                    login.removeChild(error);
                }
            }
        }
        if (ret.password !== undefined && ret.password.value !== undefined) {
            password.value = ret.password.value;
        }

        const title = document.createElement('h1');
        title.innerHTML = "Welcome " + ret.username.value + "!";

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please enter your password!';

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'login-button';
        back.onclick = () => {
            ret.password = password;
            ask_for_user();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');
        submit.innerHTML = 'Login';
        submit.className = 'login-button';
        submit.onclick = () => {
            ret.password = password;
            submit_data();
        }

        login.appendChild(title);
        login.appendChild(paragraph);
        login.appendChild(password);
        login.appendChild(document.createElement('br'));
        login.appendChild(document.createElement('br'));
        login.appendChild(back);
        login.appendChild(submit);
    }

    const ask_for_user = () => {
        const login = create_window('login-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Login';
        title.className = 'floating_window_title';
        title.id = 'login-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Welcome back to Forwarder Factory. Please enter your username or associated email address!';
        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'login-window-paragraph';

        login.appendChild(title);
        login.appendChild(paragraph);

        const username = document.createElement('input');
        username.type = 'text';
        username.name = 'username';
        username.placeholder = 'Username';
        username.className = 'login-input';
        username.id = 'login-username';
        username.onclick = () => {
            play_click();

            const errors = document.getElementsByClassName('error');
            for (let i = 0; i < errors.length; i++) {
                const error = errors[i];
                if (error.id === 'login-error') {
                    login.removeChild(error);
                }
            }
        }
        if (ret.username !== undefined && ret.username.value !== undefined) {
            username.value = ret.username.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'login-button';
        submit.onclick = () => {
            ret.username = username;
            ask_for_password();
        }

        login.appendChild(title);
        login.appendChild(paragraph);
        login.appendChild(username);
        login.appendChild(document.createElement('br'));
        login.appendChild(document.createElement('br'));
        login.appendChild(submit);

        if (_error !== "") {
            const error = document.createElement('p');
            error.innerHTML = _error;
            error.className = 'error';
            error.id = 'login-error';

            const br = document.createElement('br');
            br.className = 'error';

            login.appendChild(br);
            login.appendChild(error);
        }

        const notice = document.createElement('p');
        notice.innerHTML += "By using this service, you agree to our ";

        const terms = document.createElement('a');
        terms.innerHTML = "Terms of Service";
        terms.onclick = () => show_terms();
        terms.style.cursor = 'pointer';
        terms.style.textDecoration = 'underline';

        notice.appendChild(terms);
        login.appendChild(notice);
    }

    ask_for_user();
}

function show_register(_error = ""){
    play_click();
    set_path("/register");

    hide_initial();

    let ret = {};

    const submit_data = () => {
        play_click();

        if (ret.password.value !== ret.confirm.value) {
            show_register('Passwords do not match!');
            return;
        }

        const data = {
            username: ret.username.value,
            password: ret.password.value,
            email: ret.email.value,
        };

        fetch("/api/try_register", {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data),
        })
            .then(response => {
                if (response.status === 204) { // success without email verification needed
                    hide_all_windows();
                }

                return response.json();
            })
            .then(json => {
                if (json && json.error_str) {
                    show_register(json.error_str);
                    return;
                } else if (json && json.note && json.note === "FF_EMAIL_VERIFICATION_REQUIRED") {
                    const congrats = create_window('congrats-window');

                    const title = document.createElement('h1');
                    title.innerHTML = 'Congratulations!';
                    title.className = 'floating_window_title';
                    title.id = 'congrats-window-title';

                    const paragraph = document.createElement('p');
                    paragraph.innerHTML = 'You have successfully registered! Before you can log in, you must verify your email address. Please check your email for a verification link.';
                    paragraph.className = 'floating_window_paragraph';
                    paragraph.id = 'congrats-window-paragraph';

                    const button = document.createElement('button');
                    button.innerHTML = 'Close';
                    button.className = 'congrats-button';
                    button.onclick = () => {
                        play_click();
                        hide_all_windows();
                    }

                    congrats.appendChild(title);
                    congrats.appendChild(paragraph);
                    congrats.appendChild(button);
                }

                if (json) {
                    throw new Error('Invalid response from server: ' + JSON.stringify(json));
                }
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }

    const ask_for_confirm = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = "Welcome " + ret.username.value + "!";

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please confirm your password!';

        const confirm = document.createElement('input');
        confirm.type = 'password';
        confirm.name = 'password';
        confirm.placeholder = 'Confirm password';
        confirm.className = 'register-input';
        confirm.id = 'register-confirm-password';
        confirm.onclick = () => {
            play_click();
        }
        if (ret.confirm !== undefined && ret.confirm.value !== undefined) {
            confirm.value = ret.confirm.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.confirm = confirm;
            submit_data();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'register-button';
        back.onclick = () => {
            ret.confirm = confirm;
            ask_for_password();
        }
        back.style.marginRight = '10px';

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(confirm);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(back);
        register.appendChild(submit);
    }

    const ask_for_password = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = "Welcome " + ret.username.value + "!";

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please enter your password!';

        const password = document.createElement('input');
        password.type = 'password';
        password.name = 'password';
        password.placeholder = 'Password';
        password.className = 'register-input';
        password.id = 'register-password';
        password.onclick = () => {
            play_click();
        }
        if (ret.password !== undefined && ret.password.value !== undefined) {
            password.value = ret.password.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.password = password;
            ask_for_confirm();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'register-button';
        back.onclick = () => {
            ret.password = password;
            ask_for_username();
        }
        back.style.marginRight = '10px';

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(password);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(back);
        register.appendChild(submit);
    }

    const ask_for_username = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Register';
        title.className = 'floating_window_title';
        title.id = 'register-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please enter your desired username!';
        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'register-window-paragraph';

        const username = document.createElement('input');
        username.type = 'text';
        username.name = 'username';
        username.placeholder = 'Username';
        username.className = 'login-input';
        username.id = 'register-username';
        username.onclick = () => {
            play_click();
        }
        if (ret.username !== undefined && ret.username.value !== undefined) {
            username.value = ret.username.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'register-button';
        back.onclick = () => {
            ret.username = username;
            ask_for_email();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.username = username;
            ask_for_password();
        }

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(username);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(back);
        register.appendChild(submit);
    }

    const ask_for_email = () => {
        play_click();

        const register = create_window('register-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Register';
        title.className = 'floating_window_title';
        title.id = 'register-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Welcome to Forwarder Factory. Please enter your email address!';
        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'register-window-paragraph';

        const email = document.createElement('input');
        email.type = 'text';
        email.name = 'username';
        email.placeholder = 'Email address';
        email.className = 'login-input';
        email.id = 'register-email';
        email.onclick = () => {
            play_click();

            const errors = document.getElementsByClassName('error');
            for (let i = 0; i < errors.length; i++) {
                const error = errors[i];
                if (error.id === 'register-error') {
                    register.removeChild(error);
                }
            }
        }
        if (ret.email !== undefined && ret.email.value !== undefined) {
            email.value = ret.email.value;
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'register-button';
        submit.onclick = () => {
            ret.email = email;
            ask_for_username();
        }

        register.appendChild(title);
        register.appendChild(paragraph);
        register.appendChild(email);
        register.appendChild(document.createElement('br'));
        register.appendChild(document.createElement('br'));
        register.appendChild(submit);

        if (_error !== "") {
            const error = document.createElement('p');
            error.innerHTML = _error;
            error.className = 'error';
            error.id = 'register-error';

            const br = document.createElement('br');
            br.className = 'error';

            register.appendChild(br);
            register.appendChild(error);
        }

        const notice = document.createElement('p');
        notice.innerHTML += "By using this service, you agree to our ";

        const terms = document.createElement('a');
        terms.innerHTML = "Terms of Service";
        terms.onclick = () => show_terms();
        terms.style.cursor = 'pointer';
        terms.style.textDecoration = 'underline';

        notice.appendChild(terms);
        register.appendChild(notice);
    }

    ask_for_email();
}

function is_logged_in() {
    return cookie_exists('username');
}

function print_beta() {
    const text = document.createTextNode('Beta');
    const span = document.createElement('span');
    span.appendChild(text);
    span.className = 'beta-watermark';
    span.id = 'beta-watermark';
    span.style.position = 'absolute';
    span.style.top = '0';
    span.style.left = '0';
    span.style.padding = '10px';
    span.style.userSelect = 'none';

    document.body.appendChild(span);
}

function print_discord() {
    const url = "https://discord.gg/TuDcKUdqDS";
    const img = document.createElement('img');
    img.src = '/img/discord.svg';
    img.className = 'discord-watermark';
    img.id = 'discord-watermark';

    /* bottom left */
    img.style.position = 'absolute';
    img.style.bottom = '0';
    img.style.left = '0';
    img.style.maxWidth = '25px';
    img.style.maxHeight = '25px';
    img.style.padding = '10px';
    img.style.userSelect = 'none';
    img.style.cursor = 'pointer';
    /* on hover, scale */
    img.onmouseover = () => {
        img.style.transform = 'scale(1.1)';
    }
    img.onmouseleave = () => {
        img.style.transform = 'scale(1.0)';
    }
    img.onclick = () => {
        const w = create_window('discord-window');

        const logo = document.createElement('img');
        logo.src = '/img/discord.svg';
        logo.style.width = '50px';
        logo.style.height = '50px';

        const title = document.createElement('h1');
        title.innerHTML = 'Join our Discord server!';
        title.className = 'floating_window_title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = "Join our Discord server to chat with other users, get help, share cool things, and more."
        paragraph.innerHTML += "<br/><br/><small>Content shared on Discord does not necessarily reflect the views of Forwarder Factory. Please be aware of Discord's Terms of Service before using the service.</small>";

        w.appendChild(logo);
        w.appendChild(title);
        w.appendChild(paragraph);

        window.open(url, '_blank');
    }

    document.body.appendChild(img);
}

function print_username() {
    const username = get_cookie('username');
    if (!username) {
        return;
    }

    const text = document.createTextNode(username);
    const span = document.createElement('span');
    const i = document.createElement('i');
    i.className = 'fas fa-user';
    i.style.marginRight = '5px';

    span.appendChild(i);
    span.appendChild(text);
    span.className = 'logged-in-watermark';
    span.id = 'logged-in-watermark';
    span.style.position = 'absolute';
    span.style.top = '0';
    span.style.right = '0';
    span.style.padding = '10px';
    span.style.userSelect = 'none';

    document.body.appendChild(span);
}
function show_logout() {
    const logout = create_window('logout-window');
    const title = document.createElement('h1');
    title.innerHTML = 'Logout';
    title.className = 'floating_window_title';
    title.id = 'logout-window-title';

    const paragraph = document.createElement('p');
    paragraph.innerHTML = 'Are you sure you want to logout?';
    paragraph.className = 'floating_window_paragraph';
    paragraph.id = 'logout-window-paragraph';

    logout.appendChild(title);
    logout.appendChild(paragraph);

    const button = document.createElement('button');
    button.innerHTML = "Yes, I'm sure!";
    button.className = 'logout-button';
    button.onclick = () => {
        play_click();
        const cookies = document.cookie.split(';');
        for (let i = 0; i < cookies.length; i++) {
            const cookie = cookies[i];
            const eq = cookie.indexOf('=');
            const name = eq > -1 ? cookie.substr(0, eq).trim() : cookie.trim();
            document.cookie = `${name}=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/; domain=${window.location.hostname}`;
        }
        window.location.href = '/';
    }

    const next_button = document.createElement('button');
    next_button.style.marginLeft = '10px';
    next_button.innerHTML = 'No, regret!';
    next_button.className = 'regret-button';
    next_button.onclick = () => {
        play_click();
        hide_all_windows();
    }

    logout.appendChild(button)
    logout.appendChild(next_button);
}

function show_upload(_error = "") {
    play_click();
    set_path('/upload');

    hide_initial();

    let ret = {
        /*
        contentType,
        description,
        titleID,
        title,
        icon,
        banner,
        author,
        youtube,
        type,
        categories,
        location,
        vwiiCompatible,
        wad,
        */
    };

    const assemble_request = () => {
        play_click();

        const url = '/api/try_upload';

        // username and password does not need to be specified, as the server will check the session
        // and get the username from there
        //
        // we need to pass:
        // - file fields (banner, icon, wad)
        // - other fields (in the form of a JSON object)
        //
        // content type however is irrelevant because if we're here, we're uploading a forwarder
        let json = { meta: {} };

        if (ret.description) json.meta.description = ret.description.value;
        if (ret.titleID) json.meta.title_id = ret.titleID.value;
        if (ret.title) json.meta.title = ret.title.value;
        if (ret.author) json.meta.author = ret.author.value;
        if (ret.youtube) json.meta.youtube = ret.youtube.value;
        if (ret.type) json.meta.type = ret.type.value;
        if (ret.location) json.meta.location = ret.location.value;
        if (ret.vwiiCompatible) json.meta.vwii_compatible = ret.vwiiCompatible.value;
        if (ret.categories) json.meta.categories = ret.categories.value.split(',');

        // create a form object
        const form = new FormData();

        form.append('json', new Blob([JSON.stringify(json)], { type: 'application/json' }));

        if (ret.banner && ret.banner.files) {
            form.append('banner', ret.banner.files[0]);
        } else {
            console.error('Banner file is missing or not selected');
        }

        if (ret.icon && ret.icon.files) {
            form.append('icon', ret.icon.files[0]);
        } else {
            console.error('Icon file is missing or not selected');
        }

        if (ret.wad && ret.wad.files) {
            form.append('wad', ret.wad.files[0]);
        } else {
            console.error('WAD file is missing or not selected');
        }

        // loading screen
        const loading = create_window('loading-window', { close_button: false, moveable: false, remove_existing: true, close_on_escape: false, close_on_click_outside: false });

        const title = document.createElement('h1');
        title.innerHTML = 'Uploading...';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please wait while we upload your forwarder. This may take a few moments. Please do not close this window.';

        loading.appendChild(title);
        loading.appendChild(paragraph);

        fetch(url, {
            method: 'POST',
            body: form,
        })
            .then(response => {
                return response.json();
            })
            .then(json => {
                if (json.error_str) {
                    show_upload(json.error_str);
                    return;
                }
                if (json.id) {
                    show_forwarder(json.id);
                    return;
                }
                if (json) {
                    throw new Error('Invalid response from server: ' + JSON.stringify(json));
                }
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }
    const finalize_details = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Confirm';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = "By submitting this forwarder, you hereby pledge that you have made a good faith effort to ensure that the forwarder is not harmful to users' Wii consoles.";
        paragraph.innerHTML += " Forwarder Factory reserves the right to ban users who upload harmful, malicious or otherwise dangerous forwarders. ";
        paragraph.innerHTML += "Forwarder Factory and its members, contributors and affiliates are not responsible for any damages caused by forwarders uploaded to the site. ";
        paragraph.innerHTML += "Further, we reserve the right to remove any forwarder at any time for any reason, such as but not limited to, a DMCA takedown request.";

        const submit = document.createElement('button');

        submit.innerHTML = 'Submit';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            assemble_request();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ask_for_wad();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_wad = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'WAD File';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please upload the WAD file of the forwarder you are uploading.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'This is the file that will be installed on the Wii.';
        paragraph.innerHTML += 'Please ensure the WAD file is not corrupt as best you can. Intentionally uploading a corrupt WAD file will result in a ban.';
        paragraph.innerHTML += '<br>';

        const wad = document.createElement('input');
        wad.type = 'file';
        wad.name = 'wad';
        wad.accept = '.wad';
        wad.className = 'upload-input';
        wad.id = 'upload-wad';
        wad.style.width = '80%';
        wad.style.marginRight = '10px';
        wad.onclick = () => {
            play_click();
        }

        // TODO: some kind of serverside banner brick testing?
        // TODO 2: banner previews? someone smarter than me can do that pretty please

        const submit = document.createElement('button');
        submit.innerHTML = 'Upload';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.wad = wad;
            finalize_details();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.wad = wad;
            ask_for_vwii_compatibility();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(wad);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_vwii_compatibility = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'vWii Compatibility';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Is this forwarder compatible with the Wii U?';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += "If you are unsure, select 'Unknown'.";
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'This will be displayed to users.';

        const vwii = document.createElement('select');
        vwii.name = 'vwii';
        vwii.id = 'upload-vwii';
        vwii.className = 'upload-input';
        vwii.style.width = '100%';
        vwii.style.marginBottom = '10px';

        const options = ['Select', 'Yes', 'No', 'Unknown'];
        for (let i = 0; i < options.length; i++) {
            const option = document.createElement('option');
            option.value = options[i];
            option.innerHTML = options[i];
            vwii.appendChild(option);
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.disabled = true;
        vwii.addEventListener('input', () => {
            submit.disabled = (vwii.options[vwii.selectedIndex].value === 'Select');
        });
        submit.onclick = () => {
            ret.vwiiCompatible = vwii;
            ask_for_wad();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.vwiiCompatible = vwii;
            ask_for_type_location_categories();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(vwii);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_type_location_categories = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Type, Location, and Category';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = "We need the type of forwarder you're uploading, the location it forwards to, and the categories it belongs to.";
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'The type is the type of forwarder you are uploading. It can be a forwarder (i.e. it forwards to a program) or a channel (self contained)';

        const type = document.createElement('select');
        type.name = 'type';
        type.id = 'upload-type';
        type.className = 'upload-input';
        type.style.width = '80%';
        type.style.marginBottom = '10px';
        if (ret.type !== undefined && ret.type.value !== undefined) {
            type.value = ret.type.value;
        }

        const types = ['Select', 'Forwarder', 'Channel'];
        for (let i = 0; i < types.length; i++) {
            const option = document.createElement('option');
            option.value = types[i];
            option.innerHTML = types[i];
            type.appendChild(option);
        }

        const location = document.createElement('input');
        location.type = 'text';
        location.name = 'location';
        location.placeholder = 'Location (e.g. SD:/apps/usbloader_gx/boot.dol or USB:/apps/usbloader_gx/boot.dol or both)';
        location.className = 'upload-input';
        location.id = 'upload-location';
        location.style.width = '80%';
        location.style.marginBottom = '10px';
        location.style.display = 'none';
        if (ret.location !== undefined && ret.location.value !== undefined) {
            location.value = ret.location.value;
        }

        let unset = false;

        // if it's a forwarder, ask for location
        type.addEventListener('change', () => {
            const type = document.getElementById('upload-type');
            const selected_type = type.options[type.selectedIndex].value;
            const location = document.getElementById('upload-location');

            if (selected_type === 'Forwarder') {
                unset = false;
                location.style.display = '';
            } else if (selected_type === 'Channel') {
                unset = false;
                location.style.display = 'none';
            } else if (selected_type === 'Select') {
                unset = true;
                location.style.display = 'none';
            }
        });

        const categories = document.createElement('input');

        categories.type = 'text';
        categories.name = 'categories';
        categories.placeholder = 'Categories (comma separated)';
        categories.className = 'upload-input';
        categories.id = 'upload-categories';
        categories.style.width = '80%';
        categories.style.marginBottom = '10px';
        if (ret.categories !== undefined && ret.categories.value !== undefined) {
            categories.value = ret.categories.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.categories = categories;
            ret.location = location;
            ret.type = type;
            ask_for_youtube();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.categories = categories;
            ret.location = location;
            ret.type = type;
            ask_for_vwii_compatibility();
        }

        // disable button if type is unset
        type.addEventListener('input', () => {
            const submit = document.getElementById('continue-upload-submit');
            submit.disabled = unset;
        });

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(type);
        upload.appendChild(location);
        upload.appendChild(categories);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_youtube = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'YouTube Video (Optional)';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'If you have a YouTube video showcasing your forwarder, please provide the video URL here. This will be displayed on the forwarder page.';

        const youtube = document.createElement('input');
        youtube.type = 'text';
        youtube.name = 'youtube';
        youtube.placeholder = 'YouTube Video URL';
        youtube.className = 'upload-input';
        youtube.id = 'upload-youtube';
        youtube.style.width = '80%';
        youtube.style.marginRight = '10px';
        youtube.onclick = () => {
            play_click();
        }
        if (ret.youtube !== undefined && ret.youtube.value !== undefined) {
            youtube.value = ret.youtube.value;
        }

        // embed a preview of the video on input
        youtube.addEventListener('input', () => {
            const url = youtube.value;
            let video_id = url.split('v=')[1];
            const ampersand_position = video_id.indexOf('&');
            if (ampersand_position !== -1) {
                video_id = video_id.substring(0, ampersand_position);
            }

            // if missing youtube.com, remove the preview
            if (!url.includes('youtube.com')) {
                const preview = document.getElementById('upload-youtube-preview');
                if (preview) {
                    upload.removeChild(preview);
                }
            }

            const preview = document.getElementById('upload-youtube-preview');
            if (preview) {
                upload.removeChild(preview);
            }

            const iframe = document.createElement('iframe');
            iframe.src = `https://www.youtube.com/embed/${video_id}`;
            iframe.style.width = '100%';
            iframe.style.height = '200px';
            iframe.style.maxWidth = '50%';
            iframe.style.marginBottom = '10px';
            iframe.style.borderRadius = '5px';

            iframe.id = 'upload-youtube-preview';
            upload.appendChild(iframe);
            upload.appendChild(document.createElement('br'));

            const submit = document.getElementById('continue-upload-submit');
            const back = document.getElementById('back-upload-button');
            upload.removeChild(submit);
            upload.removeChild(back);
            upload.appendChild(back);
            upload.appendChild(submit);
        });

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.youtube = youtube;
            ask_for_author();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.youtube = youtube;
            ask_for_type_location_categories();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(youtube);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_author = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Author';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide the author of the forwarder you are uploading. This will be displayed to users.';
        paragraph.innerHTML += "If you don't know, leave it blank. Do not put your own name here, unless you are the author of the forwarder.";

        const author = document.createElement('input');
        author.type = 'text';
        author.name = 'author';
        author.placeholder = 'Author';
        author.className = 'upload-input';
        author.id = 'upload-author';
        author.style.width = '50%';
        author.style.marginBottom = '10px';
        author.onclick = () => {
            play_click();
        }

        if (ret.author !== undefined && ret.author.value !== undefined) {
            author.value = ret.author.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.style.marginRight = '10px';
        back.onclick = () => {
            ret.author = author;
            ask_for_banner();
        }

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.author = author;
            ask_for_youtube();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(author);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_banner = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Upload a Banner';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please upload a banner. This can be a screenshot or animated GIF of what is displayed upon clicking on the icon on the Wii menu.<br>This will be displayed at the top of the forwarder page.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'The banner may be any image or video file. For the best chances of your forwarder being accepted, please ensure it does not contain significant parts of the Wii menu.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'Try and capture the essence of your forwarder in a single image.';

        const banner = document.createElement('input');
        banner.type = 'file';
        banner.name = 'banner';
        banner.accept = 'image/*,video/*';
        banner.className = 'upload-input';
        banner.id = 'upload-banner';
        banner.style.width = '80%';
        banner.style.marginRight = '10px';
        banner.onclick = () => {
            play_click();
        }

        // on select, add a preview of the image on the page
        banner.addEventListener('change', () => {
            const file = banner.files[0];
            const reader = new FileReader();

            reader.onload = (e) => {
                if (file.type.includes('video')) {
                    const video = document.createElement('video');
                    video.src = e.target.result;
                    video.style.width = '100%';
                    video.style.maxWidth = '300px';
                    video.style.height = 'auto';
                    video.style.marginBottom = '10px';
                    video.style.borderRadius = '5px';
                    video.controls = true;

                    const preview = document.getElementById('upload-banner-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(video);
                    upload.appendChild(document.createElement('br'));
                    video.id = 'upload-banner-preview';
                } else {
                    const img = document.createElement('img');
                    img.src = e.target.result;
                    img.style.width = '100%';
                    img.style.maxWidth = '300px';
                    img.style.height = 'auto';
                    img.style.marginBottom = '10px';
                    img.style.borderRadius = '5px';

                    const preview = document.getElementById('upload-banner-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(img);
                    upload.appendChild(document.createElement('br'));
                    img.id = 'upload-banner-preview';
                }

                const submit = document.getElementById('continue-upload-submit');
                const back = document.getElementById('back-upload-button');
                upload.removeChild(submit);
                upload.removeChild(back);
                upload.appendChild(back);
                upload.appendChild(submit);
            }

            reader.readAsDataURL(file);
        });

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.banner = banner;
            ask_for_author();
        }
        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.banner = banner;
            ask_for_icon();
        }
        back.style.marginRight = '10px';

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(banner);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_icon = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Upload an Icon';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please upload an icon for your forwarder. This will be displayed when people are browsing for forwarders.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'The icon may be any image file. For the best chances of your forwarder being accepted, please ensure it does not contain significant parts of the Wii menu.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'Try and capture the essence of your forwarder in a single image.';
        paragraph.innerHTML += '<br>';

        const icon = document.createElement('input');
        icon.type = 'file';
        icon.name = 'icon';
        icon.accept = 'image/*,video/*';
        icon.className = 'upload-input';
        icon.id = 'upload-icon';
        icon.style.width = '80%';
        icon.style.marginRight = '10px';
        icon.onclick = () => {
            play_click();
        }

        // on select, add a preview of the image on the page
        // client side so fuck validation, worst case i guess the client can fuck up his own computer
        icon.addEventListener('change', () => {
            const file = icon.files[0];
            const reader = new FileReader();

            reader.onload = (e) => {
                if (file.type.includes('video')) {
                    const video = document.createElement('video');
                    video.src = e.target.result;
                    video.style.width = '100%';
                    video.style.maxWidth = '300px';
                    video.style.height = 'auto';
                    video.style.marginBottom = '10px';
                    video.style.borderRadius = '5px';
                    video.controls = true;

                    const preview = document.getElementById('upload-icon-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(video);
                    upload.appendChild(document.createElement('br'));
                    video.id = 'upload-icon-preview';
                } else {
                    const img = document.createElement('img');
                    img.src = e.target.result;
                    img.style.width = '100%';
                    img.style.maxWidth = '300px';
                    img.style.height = 'auto';
                    img.style.marginBottom = '10px';
                    img.style.borderRadius = '5px';

                    const preview = document.getElementById('upload-icon-preview');
                    if (preview) {
                        upload.removeChild(preview);
                    }

                    upload.appendChild(img);
                    upload.appendChild(document.createElement('br'));
                    img.id = 'upload-icon-preview';
                }

                const submit = document.getElementById('continue-upload-submit');
                const back = document.getElementById('back-upload-button');
                upload.removeChild(submit);
                upload.removeChild(back);
                upload.appendChild(back);
                upload.appendChild(submit);
            }

            reader.readAsDataURL(file);
        });

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.icon = icon;
            ask_for_banner();
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.icon = icon;
            ask_for_description();
        }
        back.style.marginRight = '10px';

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(icon);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_description = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Description';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide a description for your forwarder. This will be displayed on the forwarder page.';

        const input = document.createElement('textarea');
        input.name = 'description';
        input.placeholder = 'Description';
        input.className = 'upload-input';
        input.id = 'upload-description';
        input.style.width = '80%';
        input.style.height = '200px';
        if (ret.description !== undefined && ret.description.value !== undefined) {
            input.value = ret.description.value;
        }

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.description = input.value;
            ask_for_title();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');

        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.onclick = () => {
            ret.description = input;
            ask_for_icon();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(input);
        upload.appendChild(document.createElement('br'));
        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_title = () => {
        play_click();

        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Title and Title ID';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Please provide a title and title ID for your forwarder. The title ID is a unique identifier for your forwarder and is made up of four characters, A-Z and 0-9.';
        paragraph.innerHTML += '<br><br>';
        paragraph.innerHTML += 'The title is what will be displayed to users when they search for your forwarder. A good suggestion is what is displayed hovering over the forwarder in the Wii System Menu.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'Example: "Internet Channel" and "HAAA"';

        const title_input = document.createElement('input');
        title_input.type = 'text';
        title_input.name = 'title';
        title_input.placeholder = 'Title';
        title_input.className = 'upload-input';
        title_input.id = 'upload-title';
        title_input.style.width = '50%';
        title_input.style.marginBottom = '10px';
        title_input.onclick = () => {
            play_click();
        }
        if (ret.title !== undefined && ret.title.value !== undefined) {
            title_input.value = ret.title.value;
        }

        const title_id_input = document.createElement('input');
        title_id_input.type = 'text';
        title_id_input.name = 'title_id';
        title_id_input.placeholder = 'Title ID';
        title_id_input.className = 'upload-input';
        title_id_input.id = 'upload-title-id';
        title_id_input.style.width = '20%';
        title_id_input.style.marginLeft = '10px';
        if (ret.titleID !== undefined) {
            title_id_input.value = ret.titleID.value;
        }

        title_id_input.onclick = () => {
            play_click();
        }

        // prevent it from being too long or containing invalid characters
        title_id_input.addEventListener('input', () => {
            title_id_input.value = title_id_input.value.toUpperCase();
            title_id_input.value = title_id_input.value.replace(/[^A-Z0-9]/, '');
            title_id_input.value = title_id_input.value.substring(0, 4);

            submit.disabled = (title_input.value === '' || title_id_input.value.length !== 4 || !/^[A-Z0-9]+$/.test(title_id_input.value));
        });
        title_input.addEventListener('input', () => {
            title_input.value = title_input.value.substring(0, 30);
            submit.disabled = (title_input.value === '' || title_id_input.value.length !== 4 || !/^[A-Z0-9]+$/.test(title_id_input.value));
        });

        const back = document.createElement('button');
        back.innerHTML = 'Back';
        back.className = 'upload-button';
        back.id = 'back-upload-button';
        back.onclick = () => {
            ret.title = title_input;
            ret.titleID = title_id_input;
            ask_for_content_type();
        }
        back.style.marginRight = '10px';

        const submit = document.createElement('button');
        submit.innerHTML = 'Continue';
        submit.className = 'upload-button';
        submit.id = 'continue-upload-submit';
        submit.disabled = (title_id_input.value.length !== 4 || !/^[A-Z0-9]+$/.test(title_id_input.value));
        submit.onclick = () => {
            ret.title = title_input;
            ret.titleID = title_id_input;
            ask_for_description();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(title_input);
        upload.appendChild(title_id_input);

        upload.appendChild(document.createElement('br'));
        upload.appendChild(back);
        upload.appendChild(submit);
    }

    const ask_for_content_type = () => {
        const upload = create_window('upload-window');

        const title = document.createElement('h1');
        title.innerHTML = 'Upload';
        title.className = 'floating_window_title';
        title.id = 'upload-window-title';

        const paragraph = document.createElement('p');
        paragraph.innerHTML = 'Have some interesting content to share? Upload it here! Please note that all uploads are subject to review.';
        paragraph.innerHTML += '<br>';
        paragraph.innerHTML += 'Please ensure your forwarder is not a duplicate of an existing forwarder. ';
        paragraph.innerHTML += 'Some fields will require knowledge of the forwarder you are uploading. You can obtain all of the information required ';
        paragraph.innerHTML += 'by using a tool such as ShowMiiWads or CustomizeMii. If you are unsure about anything, please leave it blank.';

        paragraph.className = 'floating_window_paragraph';
        paragraph.id = 'upload-window-paragraph';

        const content_type = document.createElement('select');
        content_type.name = 'content_type';
        content_type.id = 'content_type';
        content_type.className = 'upload-input';
        content_type.style.width = '100%';
        content_type.style.marginBottom = '10px';

        const content_types = ['Forwarder'];
        for (let i = 0; i < content_types.length; i++) {
            const option = document.createElement('option');
            option.value = content_types[i];
            option.innerHTML = content_types[i];
            content_type.appendChild(option);
        }

        content_type.addEventListener('change', () => {
            // TODO: Maybe we can do something cool here?
            play_click();
        });

        const button = document.createElement('button');
        button.innerHTML = 'Continue';
        button.id = 'upload-continue-button';
        button.onclick = () => {
            // TODO: Handle future content types
            // if (content_type.options[content_type.selectedIndex].value === 'Forwarder') {
            ask_for_title();
        }

        upload.appendChild(title);
        upload.appendChild(paragraph);
        upload.appendChild(content_type);
        upload.appendChild(button);

        if (_error) {
            const error = document.createElement('p');
            error.classList = 'error';
            error.innerHTML = _error;
            upload.appendChild(document.createElement('br'));
            upload.appendChild(document.createElement('br'));
            upload.appendChild(error);
        }
    }

    ask_for_content_type();
}

function show_forwarder(id) {
    set_path('/view/' + id);

    hide_initial();

    const accept_forwarder = (status) => {
        const json = {
            forwarders: {
                [id]: Boolean(status)
            },
        };

        fetch('/api/set_approval_for_uploads', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(json),
        })
            .then(response => {
                if (response.status === 204) {
                    show_forwarder(id);
                }
            })
            .catch((error) => {
                console.error('Error:', error);
            });

        hide_all_windows();
    }

    const draw = (forwarder) => {
        const forwarder_window = create_window('view-window');
        forwarder_window.style.display = 'block';
        forwarder_window.style.overflowY = 'scroll';

        if (forwarder.meta.banner_type !== "video") {
            let banner = document.createElement('img');
            banner.src = `/download/${forwarder.banner_download_key}`;
            banner.className = 'view_floating_window_banner';
            banner.id = 'view_floating_window_banner';
            if (forwarder.banner_download_key) {
                forwarder_window.appendChild(banner);
            }
        } else {
            let banner = document.createElement('video');
            banner.src = `/download/${forwarder.banner_download_key}`;
            banner.className = 'view_floating_window_banner';
            banner.id = 'view_floating_window_banner';
            banner.controls = false;
            banner.autoplay = true;
            banner.loop = true;
            if (forwarder.banner_download_key) {
                forwarder_window.appendChild(banner);
            }
        }

        const title = document.createElement('h1');
        if (forwarder.meta.title) {
            title.innerHTML = forwarder.meta.title;
        } else {
            title.innerHTML = "No title provided";
        }
        title.className = 'view_floating_window_title';
        forwarder_window.appendChild(title);

        const uploader = document.createElement('p');
        uploader.className = 'view_floating_window_uploader';
        if (forwarder.uploader) {
            uploader.innerHTML = `Uploaded by ${forwarder.uploader}`;
            const fa = document.createElement('i');
            fa.className = "fa-solid fa-circle-user";
            fa.style.marginRight = '5px';
            uploader.prepend(fa);
            forwarder_window.appendChild(uploader);
        }

        const author = document.createElement('p');
        if (forwarder.meta.author) {
            author.innerHTML = `${forwarder.meta.author}`;
        } else {
            author.innerHTML = "No author provided";
        }
        author.className = 'view_floating_window_author';
        const fa = document.createElement('i');
        fa.className = 'fa-solid fa-user';
        fa.style.marginRight = '5px';
        author.prepend(fa);
        forwarder_window.appendChild(author);

        const title_id = document.createElement('p');
        if (forwarder.meta.title_id) {
            title_id.innerHTML = `${forwarder.meta.title_id}`;
            const fa = document.createElement('i');
            fa.className = 'fa-solid fa-id-card';
            fa.style.marginRight = '5px';
            title_id.prepend(fa);
            forwarder_window.appendChild(title_id);
        }

        const type = document.createElement('p');
        type.className = 'view_floating_window_type';
        if (forwarder.meta.type === 0) {
            type.innerHTML = "Forwarder";
            const fa = document.createElement('i');
            fa.className = "fa-solid fa-arrow-right";
            fa.style.marginRight = '5px';
            type.prepend(fa);
        } else {
            type.innerHTML = "Channel";
            const fa = document.createElement('i');
            fa.className = "fa-solid fa-tv";
            fa.style.marginRight = '5px';
            type.prepend(fa);
        }

        forwarder_window.appendChild(type);

        const location = document.createElement('p');
        location.className = 'view_floating_window_location';
        if (forwarder.meta.location && forwarder.meta.type === 0) {
            location.innerHTML = forwarder.meta.location;
            const fa = document.createElement('i');
            fa.className = "fa-solid fa-square-binary";
            fa.style.marginRight = '5px';
            location.prepend(fa);
            forwarder_window.appendChild(location);
        }

        const categories = document.createElement('p');
        categories.className = 'view_floating_window_categories';
        if (forwarder.meta.categories) {
            categories.innerHTML = forwarder.meta.categories.join(', ');
            const fa = document.createElement('i');
            fa.className = "fa-solid fa-tag";
            fa.style.marginRight = '5px';
            categories.prepend(fa);
            forwarder_window.appendChild(categories);
        }

        const vwii = document.createElement('p');
        vwii.className = 'view_floating_window_vwii';
        vwii.id = 'view_floating_window_vwii';
        if (forwarder.meta.vwii_compatible === 0) {
            vwii.innerHTML = "Supported: Wii";
            forwarder_window.appendChild(vwii);
        } else if (forwarder.meta.vwii_compatible === 1) {
            vwii.innerHTML = "Supported: Wii, vWii";
            forwarder_window.appendChild(vwii);
        }

        const description = document.createElement('p');
        description.className = 'view_floating_window_description';
        if (forwarder.meta.description) {
            description.innerHTML = forwarder.meta.description;
            description.id = 'view_floating_window_description';
            forwarder_window.appendChild(description);
        }

        if (forwarder.meta.youtube) {
            const iframe = document.createElement('iframe');
            iframe.src = `https://www.youtube.com/embed/${forwarder.meta.youtube}`;
            iframe.style.width = '100%';
            iframe.style.height = '400px';
            iframe.style.maxWidth = '50%';
            iframe.style.marginBottom = '10px';
            iframe.style.borderRadius = '5px';
            forwarder_window.appendChild(iframe);
        }

        const download_button = document.createElement('button');
        download_button.innerHTML = 'Download';
        download_button.className = 'view_floating_window_download';
        download_button.onclick = () => {
            window.location.href = `/download/${forwarder.data_download_key}`;
        }

        const disclaimer = document.createElement('small');
        disclaimer.className = 'view_floating_window_disclaimer';
        disclaimer.innerHTML = "Forwarder Factory is providing this forwarder as-is. Forwarder Factory and/or its contributors are not responsible for any damages caused by this file.";
        disclaimer.innerHTML += " By downloading this file, you agree to take full responsibility for any damages caused by this file. Precautions can and should be taken to prevent damage, such as using Priiloader or BootMii installed in boot2.";
        disclaimer.innerHTML += "<br/>";

        forwarder_window.appendChild(disclaimer);
        forwarder_window.appendChild(download_button);

        if (forwarder.needs_review === true && get_cookie('user_type') === '1') {
            const accept_button = document.createElement('button');
            accept_button.innerHTML = 'Accept';
            accept_button.className = 'view_floating_window_accept';
            accept_button.style.marginRight = '10px';
            accept_button.onclick = () => {
                accept_forwarder(1);
            }

            const reject_button = document.createElement('button');
            reject_button.innerHTML = 'Reject';
            reject_button.className = 'view_floating_window_reject';
            reject_button.onclick = () => {
                accept_forwarder(0);
            }

            forwarder_window.appendChild(accept_button);
            forwarder_window.appendChild(reject_button);
        }

        document.body.appendChild(forwarder_window);
    };

    const get_forwarder = async (id) => {
        const filter_data = {
            filter: {
                accepted: false,
                identifier: id,
            }
        };

        const url = '/api/get_uploads';
        const requestBody = JSON.stringify(filter_data);

        try {
            const response = await fetch(url, {
                method: 'POST',
                body: requestBody,
                headers: {
                    'Content-Type': 'application/json'
                }
            });

            const text = await response.text();

            if (text === '[]') {
                // todo: decent 404
                // for now just redir to /
                window.location.href = '/';
                return;
            }

            return JSON.parse(text);
        } catch (error) {
            console.error('Failed to get forwarder:', error);
            throw new Error('Failed to get forwarder. R u dum?');
        }
    };

    get_forwarder(id).then(_forwarder => {
        const forwarder = _forwarder.forwarders[0];
        if (!forwarder) {
            throw new Error('Forwarder not found');
        }

        draw(forwarder);
    }).catch(error => {
        console.error(error);
    });
}

function show_browse() {
    set_path('/browse');

    hide_initial();

    const title = document.getElementById('page-header');
    if (title) {
        title.style.display = 'none';
    }

    const browse = create_window('browse-window');

    const _filter_data = {
        accepted: true,
        needs_review: false,
        title: '',
        author: '',
        title_id_string: '',
        uploader: '',
        type: -1,
        categories: [],
        location: '',
        submitted_before: undefined,
        submitted_after: undefined,
        vwii: -1,
        search_string: ''
    };

    const update_previews = () => {
        const url = '/api/get_uploads';

        try {
            const requestBody = JSON.stringify({ filter: filter_data });

            fetch(url, {
                method: 'POST',
                body: requestBody,
                headers: {
                    'Content-Type': 'application/json'
                }
            })
                .then(response => response.text())
                .then(text => {
                    const json = JSON.parse(text);

                    const old = document.getElementById('browse-grid');
                    if (old) {
                        old.remove();
                    }

                    const container = document.createElement('div');
                    container.className = 'container';
                    container.id = 'browse-grid';
                    browse.appendChild(container);

                    let parent = document.createElement('div');

                    parent.className = 'grid';
                    parent.style.display = 'flex';
                    parent.style.flexDirection = 'row';
                    parent.style.flexWrap = 'wrap';
                    parent.style.gap = '10px';
                    parent.style.paddingTop = '10px';
                    parent.style.justifyContent = 'center';

                    container.appendChild(parent);

                    let item_c = 0;

                    const forwarders = json.forwarders;
                    forwarders.forEach(forwarder => {
                        const meta = forwarder.meta;
                        if (!meta) return;

                        if (item_c === 3) {
                            parent = document.createElement('div');
                            parent.className = 'grid';
                            parent.style.display = 'flex';
                            parent.style.flexDirection = 'row';
                            parent.style.flexWrap = 'wrap';
                            parent.style.gap = '10px';
                            parent.style.paddingTop = '10px';
                            parent.style.justifyContent = 'center';
                            container.appendChild(parent);
                            item_c = 0;
                        }

                        const grid = document.createElement('div');
                        grid.className = 'grid-item preview';
                        grid.id = forwarder.id;
                        grid.style.padding = '10px';
                        grid.style.flex = '1 1 50px';
                        grid.style.boxSizing = 'border-box';

                        // get page_identifier
                        const page_identifier = forwarder.page_identifier;
                        if (page_identifier) {
                            grid.onclick = () => {
                                play_click();
                                show_forwarder(page_identifier);
                            }
                        }

                        const title = document.createElement('h2');
                        title.innerHTML = meta.title;
                        title.className = 'preview-title';

                        if (title.innerHTML.length > 20) {
                            title.innerHTML = title.innerHTML.substring(0, 20) + '...';
                        }

                        const author = document.createElement('p');
                        author.innerHTML = `${meta.author}`;
                        author.className = 'preview-author';

                        let icon;
                        if (forwarder.meta.icon_type !== "video") {
                            icon = document.createElement('img');
                            icon.src = `/download/${forwarder.icon_download_key}`;
                            icon.className = 'preview-icon';
                        } else {
                            icon = document.createElement('video');
                            icon.src = `/download/${forwarder.icon_download_key}`;
                            icon.className = 'preview-icon';
                            icon.controls = false;
                            icon.autoplay = true;
                            icon.loop = true;
                            icon.muted = true;
                        }

                        const type = document.createElement('p');
                        type.className = 'preview-type';

                        let is_location = false;
                        if (meta.type === 0) {
                            type.innerHTML = "Forwarder";
                            const fa = document.createElement('i');
                            fa.className = "fa-solid fa-arrow-right";
                            fa.style.marginRight = '5px';
                            type.prepend(fa);

                            type.onclick = () => {
                                if (!is_location) {
                                    type.innerHTML = `${meta.location}`;
                                    const fa = document.createElement('i');
                                    fa.className = "fa-solid fa-square-binary";
                                    fa.style.marginRight = '5px';
                                    type.prepend(fa);
                                } else {
                                    type.innerHTML = "Forwarder";
                                    const fa = document.createElement('i');
                                    fa.className = "fa-solid fa-arrow-right";
                                    fa.style.marginRight = '5px';
                                    type.prepend(fa);
                                }
                                is_location = !is_location;
                            }
                        } else {
                            type.innerHTML = "Channel";
                            const fa = document.createElement('i');
                            fa.className = "fa-solid fa-tv";
                            fa.style.marginRight = '5px';
                            type.prepend(fa);
                        }

                        grid.appendChild(icon);
                        grid.appendChild(title);

                        if (meta.author && meta.author !== '') {
                            const fa = document.createElement('i');
                            fa.className = 'fa-solid fa-user';
                            fa.style.marginRight = '5px';
                            author.prepend(fa);
                            grid.appendChild(author);
                        }
                        grid.appendChild(type);

                        parent.appendChild(grid);
                        item_c++;
                    });

                    // if no forwarders, display a message
                    if (forwarders.size() === 0) {
                        const noResults = document.createElement('h2');
                        noResults.innerHTML = 'No results found. Why don\'t you find it for us?';
                        noResults.id = 'browse-no-results';
                        noResults.style.textAlign = 'center';
                        noResults.style.marginTop = '20px';
                        container.appendChild(noResults);
                    }

                    // append to browse window
                    browse.appendChild(container);
                })
                .catch(() => {
                    console.error('Error fetching uploads');
                });
        } catch (error) {
            console.error('Error serializing filter_data:', error);
        }
    };

    const __filter_data = {
        set(target, property, value) {
            target[property] = value;
            update_previews();
            return true;
        }
    };

    const filter_data = new Proxy(_filter_data, __filter_data);

    const toggle_filter_div = () => {
        const filter_div = document.getElementById('browse-filter-div');
        if (!filter_div) {
            const div = document.createElement('div');
            div.id = 'browse-filter-div';
            div.style.position = 'absolute';
            div.style.top = '50%';
            div.style.left = '50%';
            div.style.transform = 'translate(-50%, -50%)';
            div.style.width = '50%';
            div.style.height = '50%';
            div.style.zIndex = '99999999';
            div.style.borderTopLeftRadius = '10px';
            div.style.borderTopRightRadius = '10px';
            div.style.borderBottomLeftRadius = '10px';
            div.style.borderBottomRightRadius = '10px';
            div.style.overflow = 'scroll';

            const close = document.createElement('a');
            close.innerHTML = '✕';
            close.id = 'browse-filter-close';
            close.style.position = 'fixed';
            close.style.top = '0';
            close.style.right = '0';
            close.style.padding = '10px';
            close.style.textDecoration = 'none';
            close.style.color = 'black';
            close.onclick = () => {
                play_click();
                toggle_filter_div();
            }

            const title_filter = document.createElement('input');
            title_filter.type = 'text';
            title_filter.name = 'title_filter';
            title_filter.placeholder = 'Title (exact match)';
            title_filter.className = 'browse-filter-input';
            title_filter.id = 'browse-title-filter';
            title_filter.style.marginBottom = '10px';
            if (filter_data.title) {
                title_filter.value = filter_data.title;
            }
            title_filter.style.marginTop = '10px';
            title_filter.style.marginRight = '10px';

            title_filter.addEventListener('input', () => {
                filter_data.title = title_filter.value;
            });

            const author_filter = document.createElement('input');
            author_filter.type = 'text';
            author_filter.name = 'author_filter';
            author_filter.placeholder = 'Author (exact match)';
            author_filter.className = 'browse-filter-input';
            author_filter.id = 'browse-author-filter';
            author_filter.style.marginBottom = '10px';
            author_filter.style.marginRight = '10px';
            if (filter_data.author) {
                author_filter.value = filter_data.author;
            }

            author_filter.addEventListener('input', () => {
                filter_data.author = author_filter.value;
            });

            const title_id_filter = document.createElement('input');
            title_id_filter.type = 'text';
            title_id_filter.name = 'title_id_filter';
            title_id_filter.placeholder = 'Title ID (exact match)';
            title_id_filter.className = 'browse-filter-input';
            title_id_filter.id = 'browse-title-id-filter';
            title_id_filter.style.marginBottom = '10px';
            title_id_filter.style.marginRight = '10px';
            if (filter_data.title_id_string) {
                title_id_filter.value = filter_data.title_id_string;
            }
            title_id_filter.addEventListener('input', () => {
                title_id_filter.value = title_id_filter.value.toUpperCase();
                title_id_filter.value = title_id_filter.value.replace(/[^A-Z0-9]/, '');
                title_id_filter.value = title_id_filter.value.substring(0, 4);
                filter_data.title_id_string = title_id_filter.value;
            });

            const uploader = document.createElement('input');
            uploader.type = 'text';
            uploader.name = 'uploader';
            uploader.placeholder = 'Uploader (exact match)';
            uploader.className = 'browse-filter-input';
            uploader.id = 'browse-uploader-filter';
            uploader.style.marginBottom = '10px';
            uploader.style.marginRight = '10px';
            if (filter_data.uploader) {
                uploader.value = filter_data.uploader;
            }

            const type = document.createElement('select');
            type.name = 'type';
            type.id = 'browse-type-filter';
            type.className = 'browse-filter-input';
            type.style.marginBottom = '10px';
            type.style.marginRight = '10px';

            const types = ['Select', 'Forwarder', 'Channel'];
            for (let i = 0; i < types.length; i++) {
                const option = document.createElement('option');
                option.value = types[i];
                option.innerHTML = types[i];
                type.appendChild(option);
            }

            type.addEventListener('input', () => {
                if (type.options[type.selectedIndex].value === 'Select') {
                    filter_data.type = -1;
                }
                if (type.options[type.selectedIndex].value === 'Forwarder') {
                    filter_data.type = 0;
                }
                if (type.options[type.selectedIndex].value === 'Channel') {
                    filter_data.type = 1;
                }
            });

            if (filter_data.type !== undefined) {
                if (filter_data.type === -1) {
                    type.selectedIndex = 0;
                } else if (filter_data.type === 0) {
                    type.selectedIndex = 1;
                } else if (filter_data.type === 1) {
                    type.selectedIndex = 2;
                }
            }

            const categories = document.createElement('input');
            categories.type = 'text';
            categories.name = 'categories';
            categories.placeholder = 'Categories (comma separated)';
            categories.className = 'browse-filter-input';
            categories.id = 'browse-categories-filter';
            categories.style.marginBottom = '10px';
            categories.style.marginRight = '10px';

            if (filter_data.categories) {
                categories.value = filter_data.categories.join(', ');
            }

            categories.addEventListener('input', () => {
                filter_data.categories = categories.value.split(',').map(item => item.trim());
            });

            const location = document.createElement('input');
            location.type = 'text';
            location.name = 'location';
            location.placeholder = 'Location (exact match)';
            location.className = 'browse-filter-input';
            location.id = 'browse-location-filter';
            location.style.marginBottom = '10px';
            location.style.marginRight = '10px';
            if (filter_data.location) {
                location.value = filter_data.location;
            }

            const submitted_before = document.createElement('input');
            submitted_before.type = 'date';
            submitted_before.name = 'submitted_before';
            submitted_before.className = 'browse-filter-input';
            submitted_before.id = 'browse-submitted-before-filter';
            submitted_before.style.marginBottom = '10px';
            submitted_before.style.marginRight = '10px';
            if (filter_data.submitted_before !== undefined) {
                const date = new Date(filter_data.submitted_before);
                const year = date.getFullYear();
                const month = date.getMonth() + 1;
                const day = date.getDate();
                submitted_before.value = `${year}-${month}-${day}`;
            }
            submitted_before.addEventListener('input', () => {
                filter_data.submitted_before = new Date(submitted_before.value).getTime();
            });

            if (filter_data.type === undefined) {
                filter_data.type = -1;
            }

            const submitted_after = document.createElement('input');
            submitted_after.type = 'date';
            submitted_after.name = 'submitted_after';
            submitted_after.className = 'browse-filter-input';
            submitted_after.id = 'browse-submitted-after-filter';
            submitted_after.style.marginBottom = '10px';
            submitted_after.style.marginRight = '10px';
            if (filter_data.submitted_after !== undefined) {
                const date = new Date(filter_data.submitted_after);
                const year = date.getFullYear();
                const month = date.getMonth() + 1;
                const day = date.getDate();
                submitted_after.value = `${year}-${month}-${day}`;
            }

            submitted_after.addEventListener('input', () => {
                filter_data.submitted_after = new Date(submitted_after.value).getTime();
            });

            if (filter_data.type === undefined) {
                filter_data.type = -1;
            }

            const vwii = document.createElement('select');
            vwii.name = 'vwii';
            vwii.id = 'browse-vwii-filter';
            vwii.className = 'browse-filter-input';
            vwii.style.marginBottom = '10px';
            vwii.style.marginRight = '10px';

            const vwii_options = ['Select', 'Yes', 'No', 'Unknown'];
            for (let i = 0; i < vwii_options.length; i++) {
                const option = document.createElement('option');
                option.value = vwii_options[i];
                option.innerHTML = vwii_options[i];
                vwii.appendChild(option);
            }

            if (filter_data.vwii !== undefined) {
                if (filter_data.vwii === -1) {
                    vwii.selectedIndex = 0;
                } else if (filter_data.vwii === 1) {
                    vwii.selectedIndex = 1;
                } else if (filter_data.vwii === 2) {
                    vwii.selectedIndex = 2;
                } else if (filter_data.vwii === 0) {
                    vwii.selectedIndex = 3;
                }
            }

            vwii.addEventListener('input', () => {
                if (vwii.options[vwii.selectedIndex].value === 'Select') {
                    filter_data.vwii = -1;
                }
                if (vwii.options[vwii.selectedIndex].value === 'Yes') {
                    filter_data.vwii = 1;
                }
                if (vwii.options[vwii.selectedIndex].value === 'No') {
                    filter_data.vwii = 2;
                }
                if (vwii.options[vwii.selectedIndex].value === 'Unknown') {
                    filter_data.vwii = 0;
                }
            });

            if (filter_data.vwii === undefined) {
                filter_data.vwii = -1;
            }

            const create_label_for = (input, text) => {
                const label = document.createElement('label');
                label.innerHTML = text;
                label.htmlFor = input.id;
                label.style.marginRight = '10px';
                return label;
            }

            div.appendChild(close);

            div.appendChild(create_label_for(title_filter, 'Title'));
            div.appendChild(title_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(author_filter, 'Author'));
            div.appendChild(author_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(uploader, 'Uploader'));
            div.appendChild(uploader);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(title_id_filter, 'Title ID'));
            div.appendChild(title_id_filter);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(type, 'Type'));
            div.appendChild(type);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(categories, 'Categories'));
            div.appendChild(categories);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(location, 'Location'));
            div.appendChild(location);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(submitted_before, 'Submitted Before'));
            div.appendChild(submitted_before);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(submitted_after, 'Submitted After'));
            div.appendChild(submitted_after);
            div.appendChild(document.createElement('br'));

            div.appendChild(create_label_for(vwii, 'vWii support'));
            div.appendChild(vwii);
            div.appendChild(document.createElement('br'));

            if (get_cookie('user_type') === "1") {
                const unverified = document.createElement('input');
                unverified.type = 'checkbox';
                unverified.name = 'unverified';
                unverified.id = 'browse-unverified-filter';

                unverified.addEventListener('input', () => {
                    filter_data.needs_review = unverified.checked;
                    filter_data.accepted = !unverified.checked;
                });

                const label = document.createElement('label');
                label.innerHTML = 'Unverified';
                label.htmlFor = 'browse-unverified-filter';
                label.style.marginLeft = '10px';
                label.style.marginRight = '10px';

                div.appendChild(label);
                div.appendChild(unverified);
            }

            browse.prepend(div);
        } else {
            browse.removeChild(filter_div);
        }
    }

    const filter_button = document.createElement('button');
    filter_button.innerHTML = 'Filter';
    filter_button.className = 'browse-button';
    filter_button.id = 'browse-filter-button';
    filter_button.onclick = () => {
        play_click();
        toggle_filter_div();
    }

    const search = document.createElement('input');
    search.type = 'text';
    search.name = 'search';
    search.placeholder = 'Search';
    search.className = 'browse-input';
    search.id = 'browse-search';
    search.style.width = '50%';
    search.style.marginBottom = '10px';
    search.style.marginTop = '10px';
    search.style.marginRight = '10px';

    search.style.position = 'absolute';
    search.style.top = '1';
    search.style.left = '50%';
    search.style.transform = 'translate(-50%, 0)';

    filter_button.style.position = 'absolute';
    filter_button.style.top = '1';
    filter_button.style.left = '20%';
    filter_button.style.marginBottom = '10px';
    filter_button.style.marginTop = '10px';
    filter_button.style.marginRight = '10px';

    search.addEventListener('input', () => {
        filter_data.search_string = search.value;
    });

    update_previews();

    document.body.prepend(search);
    document.body.prepend(filter_button);
}

function show_admin() {
    set_path('/admin');
    hide_initial();

    const admin = create_window('admin-window');

    const title = document.createElement('h1');
    title.innerHTML = 'Admin Panel';
    title.className = 'admin-title';
    admin.appendChild(title);

    const unfinished = document.createElement('p');
    unfinished.innerHTML = 'This page is unfinished and is not yet functional.';
    unfinished.innerHTML += ' In the meantime, please use SQLite on the server to manage users.';
    unfinished.className = 'admin-unfinished';
    admin.appendChild(unfinished);
}
const generate_stars = (n, w) => {
    for (let i = 0; i < n; i++) {
        const star = document.createElement('div');

        star.className = 'star';
        star.style.position = 'absolute';
        star.style.width = '4px';
        star.style.height = '4px';
        star.style.backgroundColor = 'white';
        star.style.borderRadius = '50%';
        star.style.top = `${Math.random() * 100}%`;
        star.style.left = `${Math.random() * 100}%`;
        star.style.setProperty('--random-x', Math.random() - 0.5);
        star.style.setProperty('--random-y', Math.random() - 0.5);
        star.style.animationDuration = `${Math.random() * 10 + 10}s`;

        w.appendChild(star);
    }
}

function show_credits() {
    set_path('/');
    hide_initial();

    const credits = create_window('credits-window', {back_button: null, close_button: false, moveable: false, close_on_click_outside: true, close_on_escape: true});
    credits.style.overflow = "hidden";
    credits.style.minWidth = "80%";
    credits.style.minHeight = "80%";

    const roll_credits = (list, interval, restart_index = 1) => {
        let index = 0;
        let c_credit = null;

        const draw_credits = () => {
            if (index >= list.length) {
                index = restart_index;
            }

            const credit = list[index];

            const logo = document.createElement('img');
            logo.src = credit.logo;
            logo.style.marginRight = '10px';
            logo.style.zIndex = '99999';
            logo.style.width = '200px';
            logo.style.height = '200px';
            logo.style.borderRadius = '50%';
            logo.style.objectFit = 'cover';

            const name = document.createElement('h1');
            name.innerHTML = credit.name;
            name.style.color = 'white';
            name.style.zIndex = '99999';

            const role = document.createElement('h2');
            role.innerHTML = credit.role;
            role.style.color = 'white';
            role.style.zIndex = '99999';

            const container = document.createElement('div');
            container.className = 'fade-in';
            container.style.position = 'absolute';
            container.style.top = '50%';
            container.style.left = '50%';
            container.style.transform = 'translate(-50%, -50%)';
            if (credit.logo) {
                container.appendChild(logo);
            }
            if (credit.name) {
                container.appendChild(name);
            }
            if (credit.role) {
                container.appendChild(role);
            }

            if (c_credit) {
                c_credit.classList.remove('show');
                c_credit.classList.add('fade-out', 'hide');
                setTimeout(() => {
                    credits.removeChild(c_credit);
                    credits.appendChild(container);
                    requestAnimationFrame(() => {
                        container.classList.add('show');
                    });
                    c_credit = container;
                    index++;
                    setTimeout(draw_credits, interval);
                }, 1000);
            } else {
                credits.appendChild(container);
                requestAnimationFrame(() => {
                    container.classList.add('show');
                });
                c_credit = container;
                index++;
                setTimeout(draw_credits, interval, 1);
            }
        };

        draw_credits();
    };

    const list = [
        { logo: 'https://avatars.githubusercontent.com/u/88251708', name: "Forwarder Factory", role: 'Forwarder Factory was made possible by...' },
        { logo: 'https://jacobnilsson.com/img/picture.jpeg', name: 'Jacob Nilsson', role: 'Programming, Web Design, Maintenance' },
        { logo: 'https://avatars.githubusercontent.com/u/88589756', name: 'Gabubu', role: 'Maintenance' }
    ];

    generate_stars(50, credits);
    roll_credits(list, 1500);

    const blacken = document.createElement('div');
    blacken.id = 'blacken';
    blacken.style.position = 'absolute';
    blacken.style.top = '0';
    blacken.style.left = '0';
    blacken.style.width = '100%';
    blacken.style.height = '100%';
    blacken.style.backgroundColor = 'black';
    blacken.style.opacity = '0.90';
    blacken.style.zIndex = '1';

    document.body.appendChild(blacken);
}

function setup(_error = "") {
    hide_initial();

    const win = create_window('setup-window', {back_button: null, close_button: false, close_on_escape: false, close_on_click_outside: false});

    const title = document.createElement('h1');
    title.innerHTML = 'Setup';
    title.className = 'setup-title';

    const paragraph = document.createElement('p');
    paragraph.innerHTML = 'Welcome to Forwarder Factory! Before you can use the site, you must set up an admin account.';
    paragraph.innerHTML += '<br>';

    const username_input = document.createElement('input');
    username_input.type = 'text';
    username_input.name = 'username';
    username_input.placeholder = 'Username';
    username_input.className = 'setup-input';
    username_input.id = 'setup-username';
    username_input.marginRight = '10px';

    const password_input = document.createElement('input');
    password_input.type = 'password';
    password_input.name = 'password';
    password_input.placeholder = 'Password';
    password_input.className = 'setup-input';
    password_input.id = 'setup-password';
    password_input.marginRight = '10px';

    const email = document.createElement('input');
    email.type = 'email';
    email.name = 'email';
    email.placeholder = 'Email';
    email.className = 'setup-input';
    email.id = 'setup-email';
    email.marginRight = '10px';

    const submit = document.createElement('button');
    submit.innerHTML = 'Submit';
    submit.className = 'setup-button';
    submit.id = 'setup-submit';
    submit.onclick = () => {
        const json = {
            username: username_input.value,
            password: password_input.value,
            email: email.value
        };

        fetch('/try_setup', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(json),
        })
            .then(response => {
                if (response.status === 204) {
                    window.location.href = '/';
                }

                return response.json();
            })
            .then(json => {
                if (json.error_str) {
                   setup(json.error_str);
                }

                throw new Error('Failed to set up admin account');
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }

    win.appendChild(title);
    win.appendChild(paragraph);
    win.appendChild(username_input);
    win.appendChild(password_input);
    win.appendChild(email);
    win.appendChild(document.createElement('br'));

    if (_error) {
        const error = document.createElement('p');
        error.className = 'error';
        error.innerHTML = _error;
        win.appendChild(error);
    }

    win.appendChild(document.createElement('br'));
    win.appendChild(submit);
}
function get_grid(elements) {
    const container = document.createElement('div');
    container.className = 'grid';
    container.style.display = 'flex';
    container.style.flexDirection = 'row';
    container.style.flexWrap = 'wrap';
    container.style.gap = '10px';
    container.style.paddingTop = '10px';
    container.style.justifyContent = 'center';

    container.style.position = 'absolute';
    container.style.top = '50%';
    container.style.left = '50%';
    container.style.transform = 'translate(-50%, -50%)';

    elements.forEach(element => {
        element.className += ' grid-item';
        element.style.padding = '10px';
        element.style.flex = '1 1 50px';
        element.style.boxSizing = 'border-box';
        container.appendChild(element);
    });

    return container;
}

function get_link_box(p) {
    const link_box = document.createElement('div');
    link_box.className = 'link_box';

    if (p.location) {
        link_box.setAttribute('onclick', `location.href='${p.location}';`);
    } else if (p.onclick) {
        link_box.setAttribute('onclick', p.onclick);
    }

    if (p.id) {
        link_box.id = p.id;
    }

    if (p.background_color || p.color) {
        let style = '';
        if (p.background_color) {
            style += `background-color: ${p.background_color};`;
        }
        if (p.color) {
            style += `color: ${p.color};`;
        }
        link_box.setAttribute('style', style);
    }

    const title = document.createElement('h2');
    title.className = 'link_box_title';
    title.textContent = p.title;

    const description = document.createElement('p');
    description.className = 'link_box_description';
    description.textContent = p.description;

    link_box.appendChild(title);
    link_box.appendChild(description);

    return link_box;
}

function init_page() {
    let list = [];
    list.push(get_link_box({
        title: "Browse",
        description: "Browse channels uploaded by others.",
        background_color: "",
        id: "browse-button",
        onclick: "show_browse()"
    }));

    if (get_cookie("username") === null) {
        list.push(get_link_box({
            title: "Log in",
            description: "Log in to your account.",
            id: "login-button",
            onclick: "show_login()"
        }));
        list.push(get_link_box({
            title: "Register",
            description: "Register a new account.",
            id: "register-button",
            onclick: "show_register()"
        }));
    } else {
        if (get_cookie("user_type") === "1") {
            list.push(get_link_box({
                title: "Admin",
                description: "Access the admin panel.",
                id: "admin-button",
                onclick: "show_admin()"
            }));
        }
        list.push(get_link_box({
            title: "Upload",
            description: "Upload a forwarder or channel.",
            id: "upload-button",
            onclick: "show_upload()"
        }));
        list.push(get_link_box({
            title: "Log out",
            description: "Log out of your account.",
            id: "logout-button",
            onclick: "show_logout()"
        }));
    }

    list.push(get_link_box({
        title: "Credits",
        description: "View the credits for Forwarder Factory.",
        id: "credits-button",
        onclick: "show_credits()"
    }));

    const grid = get_grid(list, 'initial-link-grid');
    document.body.appendChild(grid);
}

document.addEventListener('DOMContentLoaded', () => {
    // todo: find replacement that doesn't utilize kit nonsense.
    // i hate being dependent on a third party for something as trivial as icons
    include('https://kit.fontawesome.com/aa55cd1c33.js');

    WSCBackgroundRepeatingSpawner();

    init_page();

    // required since we're not redirecting on address bar override
    window.addEventListener('popstate', () => {
        location.reload();
    });

    if (get_path() === "/login" && !is_logged_in()) show_login();
    if (get_path() === "/register" && !is_logged_in()) show_register();
    if (get_path() === "/upload" && is_logged_in()) show_upload();
    if (get_path() === "/upload" && !is_logged_in()) show_login();
    if (get_path() === "/browse") show_browse();
    if (get_path() === "/admin" && is_logged_in()) show_admin();
    if (get_path() === "/admin" && !is_logged_in()) show_login();
    if (get_path() === "/logout" && is_logged_in()) show_logout();

    if (get_path().startsWith("/view/")) {
        const id = get_path().substring(6);
        show_forwarder(id);
    }

    print_username();
    print_beta();
    print_discord();
});