pipeline {
    agent any
    environment {
        // Map Jenkins credentials to the environment for Docker Compose
        GITHUB_TOKEN = credentials('github-pat')
    }
    stages {
        stage('Compile') {
            steps {
                sh 'docker compose run --rm build'
            }
        }
        stage('Merge Binaries') {
            steps {
                sh 'docker compose run --rm merge'
            }
        }
        stage('GitHub Release') {
            when { buildingTag() }
            steps {
                // Pass the Git tag into the container
                sh "TAG_NAME=${env.TAG_NAME} docker compose run --rm release"
            }
        }
    }
    post {
        always {
            sh 'docker compose down' // Clean up containers
        }
    }
}
