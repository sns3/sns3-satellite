node('hpc') {

    stage('Delegate to build job') {
        build job: '../sns3-ci', parameters: [string(name: 'BRANCH_NAME', value: env.BRANCH_NAME), string(name: 'REPOS_NAME', value: "satellite")], propagate: true, wait: true
    }

}