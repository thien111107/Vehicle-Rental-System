// Authentication handling for login and customer registration

// Switch between Login and Register tabs
function switchAuthTab(type) {
  const loginBtn = document.getElementById('tabLoginBtn');
  const regBtn = document.getElementById('tabRegisterBtn');
  const loginForm = document.getElementById('loginForm');
  const regForm = document.getElementById('registerForm');
  const alertBox = document.getElementById('authAlert');
  alertBox.style.display = 'none';

  if (type === 'login') {
    loginBtn.classList.add('active');
    regBtn.classList.remove('active');
    loginForm.style.display = 'block';
    regForm.style.display = 'none';
  } else {
    regBtn.classList.add('active');
    loginBtn.classList.remove('active');
    regForm.style.display = 'block';
    loginForm.style.display = 'none';
  }
}

// Display alert notification box
function showAlert(messageText, isSuccess = false) {
  const alertBox = document.getElementById('authAlert');
  alertBox.textContent = messageText;
  alertBox.className = `alertBox ${isSuccess ? 'alertSuccess' : 'alertDanger'}`;
  alertBox.style.display = 'block';
}

// Quick fill helper for demo accounts
function quickFill(user, pass) {
  switchAuthTab('login');
  document.getElementById('loginUsername').value = user;
  document.getElementById('loginPassword').value = pass;
}

// Process login form submission
async function handleLogin(event) {
  event.preventDefault();
  const username = document.getElementById('loginUsername').value.trim();
  const password = document.getElementById('loginPassword').value.trim();

  const res = await api.post('/api/auth/login', { username, password });
  if (res.success) {
    api.setUser(res.data);
    showAlert('Login successful! Redirecting...', true);

    setTimeout(() => {
      const role = res.data.role;
      if (role === 'Customer') {
        window.location.href = 'customer.html';
      } else if (role === 'Manager') {
        window.location.href = 'manager.html';
      } else if (role === 'Admin') {
        window.location.href = 'admin.html';
      } else {
        alert('Invalid user role encountered!');
      }
    }, 600);
  } else {
    showAlert(res.message || 'Invalid username or password!');
  }
}

// Process customer registration form submission
async function handleRegister(event) {
  event.preventDefault();
  const username = document.getElementById('regUsername').value.trim();
  const password = document.getElementById('regPassword').value.trim();

  const res = await api.post('/api/auth/register', { username, password });
  if (res.success) {
    showAlert('Registration successful! Automatically logging in...', true);
    api.setUser(res.data);
    setTimeout(() => {
      window.location.href = 'customer.html';
    }, 800);
  } else {
    showAlert(res.message || 'Registration failed!');
  }
}
